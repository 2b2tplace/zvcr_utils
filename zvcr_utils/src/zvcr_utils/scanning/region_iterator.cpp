#include <zvcr_utils/scanning/region_iterator.hpp>
#include <charconv>

namespace zvcr {
    WrappedRegionIterator::WrappedRegionIterator(std::unique_ptr<RegionIterator> impl): impl(std::move(impl)) {}

    auto WrappedRegionIterator::operator++() -> WrappedRegionIterator& {
        impl->operator++();
        return *this;
    }

    auto WrappedRegionIterator::operator*() const -> RegionCoordinates {
        return impl->operator*();
    }

    auto WrappedRegionIterator::operator==(const WrappedRegionIterator &other) const -> bool {
        return impl->operator==(*other.impl);
    }

    RegionRectIterator::RegionRectIterator(const Rect &scanRect, const int rx, const int rz):
        scanRect(scanRect), rx(rx), rz(rz) {}

    auto RegionRectIterator::operator*() const -> RegionCoordinates {
        return RegionCoordinates { rx, rz };
    }

    auto RegionRectIterator::operator++() -> void {
        ++rz;
        if (rz >= scanRect.maxZ) {
            rz = scanRect.minZ;
            ++rx;
        }
    }

    auto RegionRectIterator::operator==(const RegionIterator &other) const -> bool {
        const auto *o = dynamic_cast<const RegionRectIterator*>(&other);
        return o && rz == o->rz && rx == o->rx;
    }

    RegionVectorIterator::RegionVectorIterator(const std::vector<RegionCoordinates> *vector, const size_t index):
        vector(vector), index(index) {}

    auto RegionVectorIterator::operator*() const -> RegionCoordinates {
        return (*vector)[index];
    }

    auto RegionVectorIterator::operator++() -> void {
        ++index;
    }

    auto RegionVectorIterator::operator==(const RegionIterator &other) const -> bool {
        const auto *o = dynamic_cast<const RegionVectorIterator*>(&other);
        return o && vector == o->vector && index == o->index;
    }

    RegionRectVectorIterator::RegionRectVectorIterator(const std::vector<Rect> *vector, const size_t index):
        vector(vector), index(index) {
        nextRect();
    }

    auto RegionRectVectorIterator::nextRect() -> bool {
        if (index >= vector->size()) {
            currentRectIterator = RegionRectIterator{};
            currentRectIteratorEnd = RegionRectIterator{};
            return false;
        }
        const auto &rect = vector->at(index);
        currentRectIterator = RegionRectIterator{rect, rect.minX, rect.minZ};
        currentRectIteratorEnd = RegionRectIterator{rect, rect.minX, rect.maxZ};
        ++index;
        return true;
    }

    auto RegionRectVectorIterator::operator*() const -> RegionCoordinates {
        return *currentRectIterator;
    }

    auto RegionRectVectorIterator::operator++() -> void {
        ++currentRectIterator;
        if (currentRectIterator == currentRectIteratorEnd)
            nextRect();
    }

    auto RegionRectVectorIterator::operator==(const RegionIterator& other) const -> bool {
        const auto *o = dynamic_cast<const RegionRectVectorIterator*>(&other);
        return o && vector == o->vector && index == o->index && currentRectIterator == o->currentRectIterator;
    }

    RegionDirectIterator::RegionDirectIterator(const fs::path &rootDir, std::string extension, const bool isEnd):
        rootDir(rootDir), extension(std::move(extension)),
        it(isEnd || !fs::is_directory(rootDir) ? fs::recursive_directory_iterator{} : fs::recursive_directory_iterator(rootDir)),
        atEnd(isEnd || !fs::is_directory(rootDir)) {
        if (!atEnd)
            advanceToNextValid();
    }

    RegionDirectIterator::RegionDirectIterator(): atEnd(true) {}

    auto RegionDirectIterator::operator*() const -> RegionCoordinates {
        return current;
    }

    auto RegionDirectIterator::operator++() -> void {
        ++it;
        advanceToNextValid();
    }

    auto RegionDirectIterator::operator==(const RegionIterator &other) const -> bool {
        const auto* o = dynamic_cast<const RegionDirectIterator*>(&other);
        return o && atEnd == o->atEnd && it == o->it;
    }

    auto RegionDirectIterator::countRegions(const fs::path &rootDir, const std::string_view extension) -> size_t {
        size_t count{};
        try {
            for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
                if (readRegionCoordinates(entry, extension))
                    count++;
            }
        } catch (const fs::filesystem_error&) {}
        return count;
    }

    auto RegionDirectIterator::readRegionCoordinates(const fs::directory_entry &entry, const std::string_view extension) -> result::Option<RegionCoordinates> {
        if (!entry.is_regular_file()) return {};
        const auto name = entry.path().filename().string();

        static constexpr std::string_view minLengthStr = "r.x.z.e";
        if (name.size() < minLengthStr.length() || name[0] != 'r' || name[1] != '.') return {};

        const auto dot1 = name.find('.', 2);
        if (dot1 == std::string::npos) return {};

        const auto dot2 = name.find('.', dot1 + 1);
        if (dot2 == std::string::npos) return {};

        const std::string_view xStr{name.data() + 2, dot1 - 2};
        const std::string_view zStr{name.data() + dot1 + 1, dot2 - dot1 - 1};
        const std::string_view extStr{name.data() + dot2 + 1,
            name.size() - dot2 - 1};

        if (extStr != extension) return {};
        int32_t x{}, z{};
        if (!parseInt(xStr, x) || !parseInt(zStr, z)) return {};

        return RegionCoordinates{x, z};
    }

    bool RegionDirectIterator::parseInt(const std::string_view s, int32_t &out) {
        const auto end = s.data() + s.size();
        auto [ptr, ec] = std::from_chars(s.data(), end, out);
        return ec == std::errc{} && ptr == end;
    }

    void RegionDirectIterator::advanceToNextValid() {
        while (it != end) {
            try {
                const auto parsed = readRegionCoordinates(*it, extension);
                if (!parsed) {
                    ++it;
                    continue;
                }
                current = *parsed;
            } catch (const fs::filesystem_error&) {
                atEnd = true;
            }
            return;
        }
        atEnd = true;
    }

    RegionMultiDirectIterator::RegionMultiDirectIterator(const std::vector<fs::path> &rootDirs, std::string extension, const bool isEnd):
        rootDirs(rootDirs),
        it(rootDirs.cbegin()),
        end(rootDirs.cend()),
        currentDirectIterator(isEnd || rootDirs.empty() ? RegionDirectIterator{} : RegionDirectIterator{rootDirs[0], extension, isEnd}),
        extension(std::move(extension)),
        atEnd(isEnd) {
        if (!atEnd)
            advanceToNextValid();
    }

    auto RegionMultiDirectIterator::operator*() const -> RegionCoordinates {
        return *currentDirectIterator;
    }

    auto RegionMultiDirectIterator::operator++() -> void {
        ++currentDirectIterator;
        advanceToNextValid();
    }

    auto RegionMultiDirectIterator::operator==(const RegionIterator &other) const -> bool {
        const auto* o = dynamic_cast<const RegionMultiDirectIterator*>(&other);
        return o && atEnd == o->atEnd;
    }

    void RegionMultiDirectIterator::advanceToNextValid() {
        while (!atEnd) {
            currentDirectIterator.advanceToNextValid();
            if (!currentDirectIterator.atEnd) {
                if (encounteredRegions.contains(*currentDirectIterator)) {
                    ++currentDirectIterator;
                    continue;
                }
                encounteredRegions.emplace(*currentDirectIterator);
                return;
            }
            if (++it == end) {
                atEnd = true;
                currentDirectIterator = RegionDirectIterator{};
                return;
            }
            currentDirectIterator = RegionDirectIterator{*it, extension, false};
        }
    }

    auto RegionMultiDirectIterator::countRegions(const std::vector<fs::path> &rootDirs, const std::string_view extension) -> size_t {
        absl::flat_hash_set<RegionCoordinates, RegionCoordinatesHash> uniqueCoordinates;
        try {
            for (const auto &rootDir : rootDirs) {
                for (const auto& entry : fs::recursive_directory_iterator(rootDir)) {
                    if (const auto coordinates = RegionDirectIterator::readRegionCoordinates(entry, extension); coordinates)
                        uniqueCoordinates.emplace(*coordinates);
                }
            }
        } catch (const fs::filesystem_error&) {}
        return uniqueCoordinates.size();
    }

    RegionVector::RegionVector(const std::vector<RegionCoordinates> &vector): vector(vector) {}

    auto RegionVector::begin() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionVectorIterator>(&vector, 0)};
    }

    auto RegionVector::end() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionVectorIterator>(&vector, vector.size())};
    }

    auto RegionVector::size() const -> size_t {
        return vector.size();
    }

    RegionRectVector::RegionRectVector(const std::vector<Rect> &vector): vector(vector) {
        for (const auto &rect : vector)
            size_ += rect.getArea();
    }

    auto RegionRectVector::begin() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionRectVectorIterator>(&vector, 0)};
    }

    auto RegionRectVector::end() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionRectVectorIterator>(&vector, vector.size())};
    }

    auto RegionRectVector::size() const -> size_t {
        return size_;
    }

    RegionDirectory::RegionDirectory(fs::path rootDir, const std::string_view extension): rootDir(std::move(rootDir)), extension(std::string{extension}) {}

    auto RegionDirectory::begin() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionDirectIterator>(rootDir, extension, false)};
    }

    auto RegionDirectory::end() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionDirectIterator>(rootDir, extension, true)};
    }

    auto RegionDirectory::size() const -> size_t {
        if (cachedSize) return *cachedSize;

        return RegionDirectIterator::countRegions(rootDir, extension);
    }

    auto RegionDirectory::invalidateCachedSize() -> void {
        cachedSize = {};
    }

    RegionMultiDirectory::RegionMultiDirectory(std::vector<fs::path> rootDirs, const std::string_view extension): rootDirs(std::move(rootDirs)), extension(std::string{extension}) {}

    auto RegionMultiDirectory::begin() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionMultiDirectIterator>(rootDirs, extension, false)};
    }

    auto RegionMultiDirectory::end() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionMultiDirectIterator>(rootDirs, extension, true)};
    }

    auto RegionMultiDirectory::size() const -> size_t {
        if (cachedSize) return *cachedSize;

        return RegionMultiDirectIterator::countRegions(rootDirs, extension);
    }

    auto RegionMultiDirectory::invalidateCachedSize() -> void {
        cachedSize = {};
    }

    RegionGrid::RegionGrid(const Rect &scanRect): scanRect(scanRect) {}

    auto RegionGrid::begin() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionRectIterator>(scanRect, scanRect.minX, scanRect.minZ)};
    }

    auto RegionGrid::end() const -> WrappedRegionIterator {
        return WrappedRegionIterator{std::make_unique<RegionRectIterator>(scanRect, scanRect.maxX, scanRect.minZ)};
    }

    auto RegionGrid::size() const -> size_t {
        return scanRect.getArea();
    }
}

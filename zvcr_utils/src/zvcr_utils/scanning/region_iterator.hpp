#pragma once

#include <vector>
#include <filesystem>
#include <result.hpp>
#include <absl/container/flat_hash_set.h>

namespace zvcr {

    struct RegionCoordinates {
        int rx;
        int rz;

        auto operator==(const RegionCoordinates& other) const noexcept -> bool {
            return rx == other.rx && rz == other.rz;
        }

        auto operator!=(const RegionCoordinates& other) const noexcept -> bool {
            return !(*this == other);
        }
    };

    struct Rect {
        int minX;
        int minZ;
        int maxX;
        int maxZ;

        [[nodiscard]]
        auto getArea() const -> uint64_t {
            return (maxX - minX) * (maxZ - minZ);
        }

        auto operator==(const Rect& other) const noexcept -> bool {
            return minX == other.minX && minZ == other.minZ && maxX == other.maxX && maxZ == other.maxZ;
        }

        auto operator!=(const Rect& other) const noexcept -> bool {
            return !(*this == other);
        }

        [[nodiscard]]
        static auto singleton(const int32_t rx, const int32_t rz) -> Rect {
            return Rect {
                .minX = rx,
                .minZ = rz,
                .maxX = rx + 1,
                .maxZ = rz + 1
            };
        }

        [[nodiscard]]
        static auto singleton(const RegionCoordinates &coordinates) -> Rect {
            return singleton(coordinates.rx, coordinates.rz);
        }
    };

    struct RegionCoordinatesHash {
        auto operator()(const RegionCoordinates& rc) const noexcept -> size_t {
            return std::hash<uint64_t>{}(static_cast<uint64_t>(static_cast<uint32_t>(rc.rx)) << 32 | static_cast<uint32_t>(rc.rz));
        }
    };

    class RegionIterator {
    public:
        virtual ~RegionIterator() = default;
        virtual auto operator*() const -> RegionCoordinates = 0;
        virtual auto operator++() -> void = 0;
        virtual auto operator==(const RegionIterator& other) const -> bool = 0;
        virtual auto operator!=(const RegionIterator& other) const -> bool {
            return !(*this == other);
        }
    };

    class WrappedRegionIterator {
    public:
        explicit WrappedRegionIterator(std::unique_ptr<RegionIterator> impl);

        auto operator++() -> WrappedRegionIterator&;

        auto operator*() const -> RegionCoordinates;

        auto operator==(const WrappedRegionIterator& other) const -> bool;

        auto operator!=(const WrappedRegionIterator& other) const -> bool {
            return !(*this == other);
        }

    private:
        std::unique_ptr<RegionIterator> impl;
    };

    class RegionRectIterator final : public RegionIterator {
        Rect scanRect{};
        int rx{};
        int rz{};

    public:
        RegionRectIterator() = default;

        explicit RegionRectIterator(const Rect &scanRect, int rx, int rz);

        [[nodiscard]]
        auto operator*() const -> RegionCoordinates override;

        auto operator++() -> void override;

        auto operator==(const RegionIterator &other) const -> bool override;
    };

    class RegionVectorIterator final : public RegionIterator {
    public:
        RegionVectorIterator(const std::vector<RegionCoordinates> *vector, size_t index);

        [[nodiscard]]
        auto operator*() const -> RegionCoordinates override;

        auto operator++() -> void override;

        auto operator==(const RegionIterator& other) const -> bool override;

    private:
        const std::vector<RegionCoordinates> *vector;
        size_t index;
    };

    class RegionRectVectorIterator final : public RegionIterator {
    public:
        RegionRectVectorIterator(const std::vector<Rect> *vector, size_t index);

        [[nodiscard]]
        auto operator*() const -> RegionCoordinates override;

        auto operator++() -> void override;

        auto operator==(const RegionIterator& other) const -> bool override;

    private:
        auto nextRect() -> bool;

        RegionRectIterator currentRectIterator{};
        RegionRectIterator currentRectIteratorEnd{};
        const std::vector<Rect> *vector;
        size_t index;
    };

    namespace fs = std::filesystem;

    class RegionDirectIterator final : public RegionIterator {
    public:
        RegionDirectIterator(const fs::path& rootDir, std::string extension, bool isEnd = false);

        RegionDirectIterator();

        [[nodiscard]]
        auto operator*() const -> RegionCoordinates override;

        auto operator++() -> void override;

        auto operator==(const RegionIterator& other) const -> bool override;

        [[nodiscard]]
        static auto countRegions(const fs::path& rootDir, std::string_view extension) -> size_t;

        [[nodiscard]]
        static auto readRegionCoordinates(const fs::directory_entry &entry, std::string_view extension) -> result::Option<RegionCoordinates>;

        auto advanceToNextValid() -> void;

        bool atEnd{false};
    private:
        fs::path rootDir;
        std::string extension;
        fs::recursive_directory_iterator it;
        fs::recursive_directory_iterator end;
        RegionCoordinates current{};

        static auto parseInt(std::string_view s, int32_t& out) -> bool;
    };

    class RegionMultiDirectIterator final : public RegionIterator {
    public:
        RegionMultiDirectIterator(const std::vector<fs::path>& rootDirs, std::string extension, bool isEnd = false);

        [[nodiscard]]
        auto operator*() const -> RegionCoordinates override;

        auto operator++() -> void override;

        auto operator==(const RegionIterator& other) const -> bool override;

        static auto countRegions(const std::vector<fs::path> &rootDirs, std::string_view extension) -> size_t;

    private:
        std::vector<fs::path> rootDirs;
        std::vector<fs::path>::const_iterator it;
        std::vector<fs::path>::const_iterator end;

        absl::flat_hash_set<RegionCoordinates, RegionCoordinatesHash> encounteredRegions;
        RegionDirectIterator currentDirectIterator;
        std::string extension;
        bool atEnd{false};

        auto advanceToNextValid() -> void;
    };

    class RegionSource {
    public:
        virtual ~RegionSource() = default;

        [[nodiscard]]
        virtual auto begin() const -> WrappedRegionIterator = 0;

        [[nodiscard]]
        virtual auto end() const -> WrappedRegionIterator = 0;

        [[nodiscard]]
        virtual auto size() const -> size_t = 0;
    };

    class RegionVector final : public RegionSource {
    public:
        explicit RegionVector(const std::vector<RegionCoordinates> &vector);

        [[nodiscard]]
        auto begin() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto end() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto size() const -> size_t override;

    private:
        std::vector<RegionCoordinates> vector;
    };

    class RegionRectVector final : public RegionSource {
    public:
        explicit RegionRectVector(const std::vector<Rect> &vector);

        [[nodiscard]]
        auto begin() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto end() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto size() const -> size_t override;

    private:
        size_t size_{};
        std::vector<Rect> vector;
    };

    class RegionDirectory final : public RegionSource {
    public:
        RegionDirectory(fs::path rootDir, std::string_view extension);

        [[nodiscard]]
        auto begin() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto end() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto size() const -> size_t override;

        auto invalidateCachedSize() -> void;

    private:
        result::Option<size_t> cachedSize;
        fs::path rootDir;
        std::string extension;
    };

    class RegionMultiDirectory final : public RegionSource {
    public:
        RegionMultiDirectory(std::vector<fs::path> rootDirs, std::string_view extension);

        [[nodiscard]]
        auto begin() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto end() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto size() const -> size_t override;

        auto invalidateCachedSize() -> void;

    private:
        result::Option<size_t> cachedSize;
        std::vector<fs::path> rootDirs;
        std::string extension;
    };

    class RegionGrid final : public RegionSource {
    public:
        explicit RegionGrid(const Rect &scanRect);

        [[nodiscard]]
        auto begin() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto end() const -> WrappedRegionIterator override;

        [[nodiscard]]
        auto size() const -> size_t override;

    private:
        Rect scanRect;
    };

}

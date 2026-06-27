#include <zvcr_utils/cli/command_line.hpp>

int main(const int argc, const char **argv) {
    if (const zvcr::CommandLine cli{argc, argv}; !cli.execute())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
/**
 * @file main.cpp
 * @brief SRRArch simulator entry point
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "logger.h"
#include "simulator.h"
#include <cstring>
#include <getopt.h>

void print_usage(const char *progname) {
  printf("Usage: %s [options] <elf-file>\n", progname);
  printf("Options:\n");
  printf("  -l, --log-level <level>  Set log level (none, error, warn, info, "
         "debug, trace)\n");
  printf("  -m, --max-insns <count>  Maximum instructions to execute (default: "
         "10000)\n");
  printf(
      "  -b, --basic-block-cache  Enable basic block cache (experimental)\n");
  printf("  -h, --help               Print this help message\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  // Default values
  srrarch::LogLevel logLevel = srrarch::LogLevel::INFO;
  uint64_t maxInstructions = 10000;
  bool useBlockCache = false;
  const char *filename = nullptr;

  // Parse command line options
  static struct option long_options[] = {
      {"log-level", required_argument, 0, 'l'},
      {"max-insns", required_argument, 0, 'm'},
      {"basic-block-cache", no_argument, 0, 'b'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "l:m:bh", long_options, nullptr)) !=
         -1) {
    switch (opt) {
    case 'l':
      if (strcmp(optarg, "none") == 0)
        logLevel = srrarch::LogLevel::NONE;
      else if (strcmp(optarg, "error") == 0)
        logLevel = srrarch::LogLevel::ERROR;
      else if (strcmp(optarg, "warn") == 0)
        logLevel = srrarch::LogLevel::WARNING;
      else if (strcmp(optarg, "info") == 0)
        logLevel = srrarch::LogLevel::INFO;
      else if (strcmp(optarg, "debug") == 0)
        logLevel = srrarch::LogLevel::DBG;
      else if (strcmp(optarg, "trace") == 0)
        logLevel = srrarch::LogLevel::TRACE;
      else {
        fprintf(stderr, "Invalid log level: %s\n", optarg);
        return 1;
      }
      break;
    case 'm':
      maxInstructions = strtoull(optarg, nullptr, 10);
      break;
    case 'b':
      useBlockCache = true;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: No ELF file specified\n");
    print_usage(argv[0]);
    return 1;
  }

  filename = argv[optind];

  // Set log level
  srrarch::Logger::instance().setLevel(logLevel);

  LOG_INFO("Log level set to %d", static_cast<int>(logLevel));
  LOG_INFO("Max instructions: %lu", maxInstructions);
  if (useBlockCache) {
    LOG_INFO("Basic block cache enabled");
  } else {
    LOG_INFO("Basic block cache disabled (using interpreter)");
  }

  srrarch::Simulator sim;
  sim.set_max_instructions(maxInstructions);
  sim.set_use_block_cache(useBlockCache);

  srrarch::LoadResult result = sim.load_elf(filename);
  if (result == srrarch::LoadResult::SUCCESS) {
    sim.run();
    return 0;
  } else {
    LOG_ERROR("Failed to load ELF: %s", srrarch::load_result_to_string(result));
    return 1;
  }
}
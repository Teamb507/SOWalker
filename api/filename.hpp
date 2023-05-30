#ifndef GRAPHWALKER_FILENAMES_DEF
#define GRAPHWALKER_FILENAMES_DEF

#include <fstream>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "logger/logger.hpp"

/**
 * Configuration file name
 */
static std::string configname() {
    char * chi_root = getenv("RANDGRAPH_ROOT");
    if (chi_root != NULL) {
        return std::string(chi_root) + "/conf/randgraph.cnf";
    } else {
        return "conf/randgraph.cnf";
    }
}

/**
 * Configuration file name - local version which can
 * override the version in the version control.
 */
static std::string configlocalname() {
    char *chi_root = getenv("RANDGRAPH_ROOT");
    if (chi_root != NULL) {
        return std::string(chi_root) + "/conf/randgraph.local.cnf";
    } else {
        return "conf/randgraph.local.cnf";
    }
}

#endif
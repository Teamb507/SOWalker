#include "preprocess/graph_converter.hpp"
#include "engine/config.hpp"
#include "api/cmdopts.hpp"

int main(int argc, const char* argv[]) {
    assert(argc >= 2);
    set_argc(argc, argv);
    logstream(LOG_INFO) << "app : " << argv[0] << ", dataset : " << argv[1] << std::endl;
    std::string input = argv[1];
    bool weighted = get_option_bool("weighted");
    bool sorted   = get_option_bool("sorted");
    graph_converter converter(remove_extension(input), weighted, sorted);
    auto query_blocksize = [](vid_t nvertices){return BLOCK_SIZE;};
    convert(input, converter, query_blocksize);
    logstream(LOG_INFO) << "  ================= FINISHED ======================  " << std::endl;
    return 0;
}

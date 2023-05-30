#ifndef _GRAPH_REPORTER_H_
#define _GRAPH_REPORTER_H_

#include "reps/basic_reporter.hpp"
#include "reps/file_reporter.hpp"
#include "reps/html_reporter.hpp"

/**
    * Helper for metrics.
    */
static VARIABLE_IS_NOT_USED void metrics_report(metrics &m);
static VARIABLE_IS_NOT_USED void metrics_report(metrics &m)
{
    // std::string reporters = get_option_string("metrics.reporter", "console, file, html");
    std::string reporters = "console,file";
    char *creps = (char *)reporters.c_str();
    const char *delims = ",";
    char *t = strtok(creps, delims);

    while (t != NULL)
    {
        std::string repname(t);
        if (repname == "basic" || repname == "console")
        {
            basic_reporter rep;
            m.report(rep);
        }
        else if (repname == "file")
        {
            file_reporter rep(get_option_string("metrics.reporter.filename", "randgraph_metrics.txt"));
            m.report(rep);
        }
        else if (repname == "html")
        {
            html_reporter rep(get_option_string("metrics.reporter.htmlfile", "randgraph_metrics.html"));
            m.report(rep);
        }
        else
        {
            logstream(LOG_WARNING) << "Could not find metrics reporter with name [" << repname << "], ignoring." << std::endl;
        }
        t = strtok(NULL, delims);
    }
}

#endif
#pragma once

struct cfg_error
        : std::runtime_error
{
    cfg_error(std::string err)
        :  runtime_error(err)
    {
    }
};

struct tunnel;
void parse_config(string filename, tunnel& t);

#include <cpr/cpr.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>
#include <cli/cli.h>
#include <cli/loopscheduler.h>
#include <cli/clilocalsession.h>
#include <memory>
#include <stdexcept>

using json = nlohmann::json;

struct SlId
{
    int id;
    std::string slug;
};



// writes a file to FS, wrapper performs error checks
void writeFile(const std::string name, const std::string text)
{
    std::string outputStr = name + ".json";

    std::ofstream output;
    try
    {
        output.open(outputStr);
        output << text;
        output.close();
    } catch (std::ios_base::failure& err)
    {
        std::string errMsg = std::string("Failed to write file: ").append(err.what());
        throw std::runtime_error(errMsg);
    }

    std::cout<<outputStr<<" succesfully written.\n";
}

// wrapper to check a curl/cpr response for problems
bool checkGR(cpr::Response response)
{
    if (response.error)
    {
        std::cerr<<"Network Error : "<< static_cast<int>(response.error.code) << " : " << response.error.message<<"\n";
        return false;
    }

    if (response.status_code >= 400)
    {
        std::cerr<<"HTTP Error : "<< response.status_code<<"\n";
        return false;
    }

    return true;
}

// check whether a Id is a 'real' army and not mercs/reinf
bool checkPlayable(const int id)
{
    int last2 = id % 100;

    if (last2 == 99)
        return false;
    else
        return true;
}

// downloads a faction datafile from CB API and writes to json
int factionGet(int id, const std::string fac)
{
    std::string idStr = std::to_string(id);

    std::string url = "https://api.corvusbelli.com/army/units/en/";

    url.append(idStr);

    cpr::Response faction = cpr::Get(cpr::Url{url},
                                    cpr::Header{{"Origin","https://infinityuniverse.com/"}});

    if (checkGR(faction))
        writeFile(fac, faction.text);

    return 0;
}

// uses cli to provide a user interface for selecting and downloading faction dataFiles as well as meta+list functionality
int armyGetSelect()
{
    cpr::Response meta = cpr::Get(cpr::Url{"https://api.corvusbelli.com/army/infinity/en/metadata"},
                                    cpr::Header{{"Origin","https://infinityuniverse.com/"}});

    if (checkGR(meta)==false)
        return 1;

    auto jsonMeta = json::parse(meta.text);

    json factions = jsonMeta["factions"];

    std::vector<SlId> facList;

    auto factionMenu = std::make_unique<cli::Menu>("faction");

    // populate factions
    {
        // iterate through factions
        for (const auto& faction : factions)
        {
            // filter out non-playables
            if (checkPlayable(faction["id"].get<int>()))
            {
                int id = faction["id"];
                std::string slug = faction["slug"];

                // callback fx
                auto fx = [id, slug](std::ostream& out)
                {
                    factionGet(id,slug);
                };

                factionMenu->Insert(slug, fx, faction["name"]);
                SlId newSlid = {id, slug};
                facList.push_back(newSlid);
            }
        }

    }

    auto topMenu = std::make_unique<cli::Menu>("infArmyGet");

    topMenu->Insert(std::move(factionMenu));

    // populate the list command
    {
        auto listFx = [facList](std::ostream& out)
        {
            for (const auto& slid : facList)
                std::cout<<slid.id<<" : "<<slid.slug<<"\n";
        };

        topMenu->Insert("list", listFx, "List available factions");
    }

    // output metadata
    {
        std::string text = meta.text;
        auto metaFx = [text](std::ostream& out)
        {
            writeFile("meta", text);
        };

        topMenu->Insert("meta", metaFx, "Get army metadata");
    }


    cli::Cli myCli(std::move(topMenu));

    cli::LoopScheduler sched;

    auto closeFx = [&sched](std::ostream& out)
    {
        sched.Stop();
    };
    myCli.ExitAction(closeFx);

    cli::CliLocalTerminalSession localSession(myCli, sched, std::cout);

    sched.Run();

    return 0;
}


int optList()
{
    cpr::Response meta = cpr::Get(cpr::Url{"https://api.corvusbelli.com/army/infinity/en/metadata"},
                                  cpr::Header{{"Origin","https://infinityuniverse.com/"}});

    if (checkGR(meta)==false)
        return 1;

    auto jsonMeta = json::parse(meta.text);

    json factions = jsonMeta["factions"];

    // populate factions
    {
        // iterate through factions
        for (const auto& faction : factions)
        {
            // filter out non-playables
            if (checkPlayable(faction["id"].get<int>()))
            {
                int id = faction["id"];
                std::string slug = faction["slug"];

                std::cout<<id<<" : "<<slug<<"\n";
            }
        }

    }

    return 0;
}

int optMeta()
{
    cpr::Response meta = cpr::Get(cpr::Url{"https://api.corvusbelli.com/army/infinity/en/metadata"},
                                  cpr::Header{{"Origin","https://infinityuniverse.com/"}});

    if (checkGR(meta)==false)
        return 1;

    writeFile("meta", meta.text);

    return 0;
}

int optById(const std::vector<int> selIDs)
{
    cpr::Response meta = cpr::Get(cpr::Url{"https://api.corvusbelli.com/army/infinity/en/metadata"},
                                  cpr::Header{{"Origin","https://infinityuniverse.com/"}});

    if (checkGR(meta)==false)
        return 1;

    auto jsonMeta = json::parse(meta.text);

    json factions = jsonMeta["factions"];

    std::map<int, std::string> checkMap;

    for (const auto& faction : factions)
    {
        // filter out non-playables
        if (checkPlayable(faction["id"].get<int>()))
        {
            int id = faction["id"];
            std::string slug = faction["slug"];

            checkMap.insert({id,slug});
        }
    }

    for (auto& cid : selIDs)
    {
        if (checkMap.contains(cid))
        {
            factionGet(cid,checkMap[cid]);
        }
        else
        {
            std::cout<<cid<<" is not a valid army id.\n";
        }
    }

    return 0;
}


int main(int argc, char ** argv)
{
    argparse::ArgumentParser parser("infArmyGet");

    parser.add_description("Infinity Army Getter - CLI tool for downloading army data.\nTo launch UI do not use any arguments.\n");

    parser.add_argument("-id").help("Enter ID(s) of faction as program argument(s) to skip UI").nargs(argparse::nargs_pattern::any).scan<'i',int>();
    parser.add_argument("-meta").help("Download and save army metadata").implicit_value(true).default_value(false);
    parser.add_argument("-list").help("Print list of factions with IDs").implicit_value(true).default_value(false);

    try
    {
        parser.parse_args(argc,argv);
    }
    catch (const std::exception& err)
    {
        std::cerr<< err.what() <<"\n";
        std::cerr<< parser;
        std::exit(1);
    }

    if (parser.is_used("-id") == false && parser.is_used("-meta") == false && parser.is_used("-list") == false)
    {
        armyGetSelect();
    }
    else
    {
        if (parser.is_used("-list") == true)
        {
            optList();
        }
        if (parser.is_used("-meta") == true)
        {
            optMeta();
        }
        if (parser.is_used("-id") == true)
        {
            auto vals = parser.get<std::vector<int>>("-id");
            optById(vals);
        }
    }


    return 0;
}




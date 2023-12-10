#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace BMS {
    struct BPMInfo
    {
        int    Measure;
        double Offset;
        double Value;
    };

    struct STOPInfo
    {
        int    Measure;
        double Offset;
        double Value;
    };

    struct BMSEvent
    {
        double Measure;
        double Position;
        int    Channel;
        double Value;
    };

    struct BMSTiming
    {
        double StartTime;
        double Value;
        float  TimeSignature;
    };

    struct BMSNote
    {
        double StartTime;
        double EndTime;
        int    Lane;
        int    SampleIndex;
    };

    struct BMSAutoSample
    {
        double StartTime;
        int    SampleIndex;
    };

    class BMSFile
    {
    public:
        BMSFile();
        ~BMSFile();

        void Load(std::filesystem::path &path);
        bool IsValid();

        std::string           Title = "";
        std::string           Artist = "";
        std::string           StageFile = "";
        std::filesystem::path CurrentDir;
        int                   Level = 0;
        float                 BPM = 130.0f;
        double                AudioLength = 0;

        std::vector<BMSNote>       Notes;
        std::vector<BMSTiming>     Timings;
        std::vector<BMSAutoSample> AutoSamples;
        std::vector<double>        Measures;
        std::map<int, std::string> Samples;

    private:
        void CompileData(std::vector<std::string> &lines);
        void CompileNoteData();
        void VerifyNote();
        int  GetSampleIndex(std::string msg);

        bool        m_valid = false;
        std::string m_lnObj;
        int         m_lnType;

        std::map<int, std::vector<BMSNote>>              m_perLaneNotes;
        std::vector<std::pair<std::string, std::string>> m_wavs;
        std::unordered_map<std::string, double>          m_bpms;
        std::unordered_map<std::string, double>          m_stops;
        std::vector<BMSEvent>                            m_events;
    };
} // namespace BMS
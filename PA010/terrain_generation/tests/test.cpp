#include "../application.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <cassert>
#include <functional>
#include <filesystem>

struct TestSample
{
    glm::vec3 pos;
    float result;
};

struct TestInfo
{
    std::string  method_name;
    Application::SpectralSystesisInfo synthesis_info;
    std::vector<TestSample> samples;
};

static void save(std::ofstream& ostr, Application::SpectralSystesisInfo const& info) {
    ostr << info.noise_frequency << '\n';
    ostr << info.sampling_frequency << '\n';
    ostr << info.start_amplitude << '\n';
    ostr << info.amplitude_multiplier << '\n';
    ostr << info.frequency_multiplier << '\n';
    ostr << info.arg0 << '\n';
}

static void save(std::ofstream& ostr, TestSample const& sample) {
    ostr << sample.pos.x << '\n';
    ostr << sample.pos.y << '\n';
    ostr << sample.pos.z << '\n';
    ostr << sample.result << '\n';
}

static void save(std::ofstream& ostr, TestInfo const& info) {
    ostr << info.method_name << '\n';
    save(ostr, info.synthesis_info);
    ostr << info.samples.size() << '\n';
    for (TestSample const& sample : info.samples)
        save(ostr, sample);
}

std::string load_string(std::ifstream& istr) {
    std::string line;
    const bool validity = !std::getline(istr, line).fail();
    EXPECT_TRUE(validity) << "Failed to read a line from the test file.";
    return line;
}

int load_int(std::ifstream& istr) { return std::stoi(load_string(istr)); }
unsigned int load_uint(std::ifstream& istr) { return static_cast<unsigned int>(load_int(istr)); };
float load_float(std::ifstream& istr) { return std::stof(load_string(istr)); }

static void load(std::ifstream& istr, Application::SpectralSystesisInfo& info) {
    info.noise_frequency = load_float(istr);
    info.sampling_frequency = load_uint(istr);
    info.start_amplitude = load_float(istr);
    info.amplitude_multiplier = load_float(istr);
    info.frequency_multiplier = load_float(istr);
    info.arg0 = load_float(istr);
}

static void load(std::ifstream& istr, TestSample& sample) {
    sample.pos.x = load_float(istr);
    sample.pos.y = load_float(istr);
    sample.pos.z = load_float(istr);
    sample.result = load_float(istr);
}

static void load(std::ifstream& istr, TestInfo& info) {
    info.method_name = load_string(istr);
    load(istr, info.synthesis_info);
    unsigned int const num_samples = load_uint(istr);
    EXPECT_NE(num_samples, 0U);
    for (unsigned int i = 0UL; i < num_samples; ++i) {
        info.samples.push_back({});
        load(istr, info.samples.back());
    }
}

static std::filesystem::path data_dir;
static bool regenerate = false;

static void save_noises() {
    std::vector<Application::SpectralSystesisInfo> spectral_infos;
    {
        spectral_infos.push_back({});

        spectral_infos.push_back({});
        spectral_infos.back().noise_frequency = 2.0f;
        spectral_infos.back().frequency_multiplier = 2.17f;
        spectral_infos.back().start_amplitude = 1.25f;
        spectral_infos.back().amplitude_multiplier = 0.75f;
        spectral_infos.back().arg0 = 20.0f;

        spectral_infos.push_back({});
        spectral_infos.back().noise_frequency = 10.0f;
        spectral_infos.back().frequency_multiplier = 1.75f;
        spectral_infos.back().start_amplitude = 2.5f;
        spectral_infos.back().amplitude_multiplier = 0.4f;
        spectral_infos.back().arg0 = 5.0f;
    }

    for (int i = 0; i < (int)spectral_infos.size(); ++i)
        for (auto const& name_and_generator : Application::noise_functions_map)
        {
            TestInfo info;
            info.synthesis_info = spectral_infos.at(i);
            info.method_name = name_and_generator.first;
            for (float z = -1.0f; z <= 1.0f; z += 0.25f)
                for (float y = -1.0f; y <= 1.0f; y += 0.25f)
                    for (float x = -1.0f; x <= 1.0f; x += 0.25f)
                    {
                        glm::vec3 const pos(x,y,z);
                        info.samples.push_back({ pos, name_and_generator.second(pos, info.synthesis_info) });
                    }
            std::stringstream sstr;
            sstr << info.method_name << '.' << i << ".txt";
            std::ofstream ostr((data_dir / sstr.str()).string());
            save(ostr, info);
        }
}

static void test_noises() {
    unsigned int num_processed_tests = 0U;
    for (auto const& dir_entry : std::filesystem::directory_iterator(data_dir))
        if (std::filesystem::is_regular_file(dir_entry.path()))
        {
            TestInfo info;
            std::ifstream istr(dir_entry.path().string()); 
            load(istr, info);

            for (TestSample const& sample : info.samples)
            {
                float const tested_result = Application::noise_functions_map.at(info.method_name)(sample.pos, info.synthesis_info);
                EXPECT_LE(fabs(tested_result - sample.result), 0.001f)
                    << "Incorrect rasult " << tested_result << " (correct is " << sample.result << ") was obtained at position ["
                    << sample.pos.x << ',' << sample.pos.y << ',' << sample.pos.z << "] from the method '" << info.method_name
                    << "' for this SpectralSystesisInfo{"
                    << "noise_frequency=" << info.synthesis_info.noise_frequency
                    << ",sampling_frequency=" << info.synthesis_info.sampling_frequency
                    << ",start_amplitude=" << info.synthesis_info.start_amplitude
                    << ",frequency_multiplier=" << info.synthesis_info.frequency_multiplier
                    << ",arg0=" << info.synthesis_info.arg0
                    << "}.";
            }

            ++num_processed_tests;
        }
    EXPECT_NE(num_processed_tests, 0U);
}

TEST(NoiseTestSuite, noise_test) {
    if (regenerate)
        save_noises();
    test_noises();
}

int main(int argc, char** argv) {
    data_dir = Configuration(argv[0]).get_path("data");
    regenerate = argc >= 2 && strncmp(argv[argc - 1], "--generate", 10) == 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

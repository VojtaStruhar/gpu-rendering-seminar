#include "application.hpp"
#include <cassert>

// For a passed lattice point p returns the corresponding gradient vector.
// IMPORTANT: Any lattice point has all coordinates integers, i.e. the fractional parts are 0.
//            That means, all p.x, p.y, and p.z  must be from { ... -2.0, -1.0, 0.0, 1.0, 2.0 ... }.
//            Given a real number x, you can obtain its integral part, for example, using the function "std::floor(x)".
// NOTE: This method is fully implemented, so do not modify its code. You are only supposed to use
//       it when implementing the function "perlin_noise_improved" below.
glm::vec3 get_gradient(glm::vec3 p) {
    constexpr unsigned int NUM_INDICES = 256U;
    static const unsigned int permutation[NUM_INDICES] = {
            225U, 155U, 210U, 108U, 175U, 199U, 221U, 144U, 203U, 116U, 70U, 213U, 69U, 158U, 33U, 252U,
            5U, 82U, 173U, 133U, 222U, 139U, 174U, 27U, 9U, 71U, 90U, 246U, 75U, 130U, 91U, 191U,
            169U, 138U, 2U, 151U, 194U, 235U, 81U, 7U, 25U, 113U, 228U, 159U, 205U, 253U, 134U, 142U,
            248U, 65U, 224U, 217U, 22U, 121U, 229U, 63U, 89U, 103U, 96U, 104U, 156U, 17U, 201U, 129U,
            36U, 8U, 165U, 110U, 237U, 117U, 231U, 56U, 132U, 211U, 152U, 20U, 181U, 111U, 239U, 218U,
            170U, 163U, 51U, 172U, 157U, 47U, 80U, 212U, 176U, 250U, 87U, 49U, 99U, 242U, 136U, 189U,
            162U, 115U, 44U, 43U, 124U, 94U, 150U, 16U, 141U, 247U, 32U, 10U, 198U, 223U, 255U, 72U,
            53U, 131U, 84U, 57U, 220U, 197U, 58U, 50U, 208U, 11U, 241U, 28U, 3U, 192U, 62U, 202U,
            18U, 215U, 153U, 24U, 76U, 41U, 15U, 179U, 39U, 46U, 55U, 6U, 128U, 167U, 23U, 188U,
            106U, 34U, 187U, 140U, 164U, 73U, 112U, 182U, 244U, 195U, 227U, 13U, 35U, 77U, 196U, 185U,
            26U, 200U, 226U, 119U, 31U, 123U, 168U, 125U, 249U, 68U, 183U, 230U, 177U, 135U, 160U, 180U,
            12U, 1U, 243U, 148U, 102U, 166U, 38U, 238U, 251U, 37U, 240U, 126U, 64U, 74U, 161U, 40U,
            184U, 149U, 171U, 178U, 101U, 66U, 29U, 59U, 146U, 61U, 254U, 107U, 42U, 86U, 154U, 4U,
            236U, 232U, 120U, 21U, 233U, 209U, 45U, 98U, 193U, 114U, 78U, 19U, 206U, 14U, 118U, 127U,
            48U, 79U, 147U, 85U, 30U, 207U, 219U, 54U, 88U, 234U, 190U, 122U, 95U, 67U, 143U, 109U,
            137U, 214U, 145U, 93U, 92U, 100U, 245U, 0U, 216U, 186U, 60U, 83U, 105U, 97U, 204U, 52U
    };
    static auto const get_permutation = [](float c) -> unsigned int {
        return permutation[((unsigned int) fabs(c)) & (NUM_INDICES - 1U)];
    };
    static auto const get_permuted_index = [](glm::vec3 p) -> unsigned int {
        return get_permutation(p.z + (float) get_permutation(p.y + (float) get_permutation(p.x)));
    };

    constexpr unsigned int NUM_GRADIENTS = 16U;
    static const glm::vec3 gradient[NUM_GRADIENTS] = {
            glm::vec3(-1, -1, 0), glm::vec3(1, 1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0),
            glm::vec3(-1, 0, -1), glm::vec3(1, 0, 1), glm::vec3(-1, 0, 1), glm::vec3(1, 0, -1),
            glm::vec3(0, -1, -1), glm::vec3(0, 1, 1), glm::vec3(0, -1, 0), glm::vec3(0, 1, -1),
            glm::vec3(1, 1, 0), glm::vec3(0, -1, 1), glm::vec3(0, -1, -1), glm::vec3(-1, 1, 0)
    };

    return gradient[get_permuted_index(p) & (NUM_GRADIENTS - 1U)];
}


//
// YOUR HOMEWORK IS TO IMPLEMENT ALL FOUR FUNCTIONS BELOW.
//


// IMPORTANT: A noise value computed by any of the functions below should NOT be clamped to <-1,1>. I.e. just return the computed value as it is.
//            Clamping is automatically done later by the framework (when building colours of the noise texture).
// IMPORTANT: For the spectral synthesis noise functions (fractal sum, turbulence, and marble) sum octaves according to the Nyquist theorem.
//            That is, while "2.0f * frequency_of_current_octave <= info.sampling_frequency" is true.
// IMPORTANT: For details about the functionality of all four functions below see slides of the Lecture #6 "Advanced Texturing".
//            Most relevant slides are marked by the label "HOMEWORK HELP".


// Implementation of the "3D Perlin Noise Improved" noise function.
float Application::perlin_noise_improved(glm::vec3 pos) {
    // TODO: Implement this function.
    // HINTS:
    //  - glm::dot(u,v) - Computes the "dot product" of the passed vectors "u" and "v".
    //  - std::floor(x) - Accepts a real number "x" and returns its rounded down value,
    //                    e.g. std::floor(1.3f)=1.0f, std::floor(1.8f)=1.0f, std::floor(-1.3f)=-2.0f, std::floor(-1.8f)=-2.0f.
    //  - get_gradient(p) - Returns a gradient vector for a passed lattice point p. For more details read the comment of that
    //                      function (it is defined above).
    //  - You need to implement the interpolation polynomial s(t) of the degree 5 (see the lecture). 
    //    Consider to do so in a separate function (that you can define above this function):
    //          float smooth_step(float t) { return ...; }
    //  - You also need to implement the linear interpolation of two scalar values (in particular, two dot products; see the lecture).
    //    Consider to do so in a separate function that interpolates linearly between numbers "a" and "b" using a interpolation parameter "t". 
    //    (that you can define above this function):
    //          float mix(float a, float b, float t) { return ...; }


    // Rounded position (lattice?)
    float xi = std::floor(pos.x);
    float yi = std::floor(pos.y);
    float zi = std::floor(pos.z);

    // Offsets from lattice points
    float xf = pos.x - xi;
    float yf = pos.y - yi;
    float zf = pos.z - zi;

    float u = smooth_step(xf);
    float v = smooth_step(yf);
    float w = smooth_step(zf);

    // Lattice gradients
    glm::vec3 grad000 = get_gradient(glm::vec3(xi, yi, zi));
    glm::vec3 grad001 = get_gradient(glm::vec3(xi, yi, zi + 1));
    glm::vec3 grad010 = get_gradient(glm::vec3(xi, yi + 1, zi));
    glm::vec3 grad011 = get_gradient(glm::vec3(xi, yi + 1, zi + 1));
    glm::vec3 grad100 = get_gradient(glm::vec3(xi + 1, yi, zi));
    glm::vec3 grad101 = get_gradient(glm::vec3(xi + 1, yi, zi + 1));
    glm::vec3 grad110 = get_gradient(glm::vec3(xi + 1, yi + 1, zi));
    glm::vec3 grad111 = get_gradient(glm::vec3(xi + 1, yi + 1, zi + 1));

    // Compute the dot product between the gradients and the vectors from the points to the lattice points
    float dot000 = glm::dot(grad000, glm::vec3(xf, yf, zf));
    float dot001 = glm::dot(grad001, glm::vec3(xf, yf, zf - 1));
    float dot010 = glm::dot(grad010, glm::vec3(xf, yf - 1, zf));
    float dot011 = glm::dot(grad011, glm::vec3(xf, yf - 1, zf - 1));
    float dot100 = glm::dot(grad100, glm::vec3(xf - 1, yf, zf));
    float dot101 = glm::dot(grad101, glm::vec3(xf - 1, yf, zf - 1));
    float dot110 = glm::dot(grad110, glm::vec3(xf - 1, yf - 1, zf));
    float dot111 = glm::dot(grad111, glm::vec3(xf - 1, yf - 1, zf - 1));

    float lerp_x1 = mix(dot000, dot100, u);
    float lerp_x2 = mix(dot010, dot110, u);
    float lerp_x3 = mix(dot001, dot101, u);
    float lerp_x4 = mix(dot011, dot111, u);

    float lerp_y1 = mix(lerp_x1, lerp_x2, v);
    float lerp_y2 = mix(lerp_x3, lerp_x4, v);

    float result = mix(lerp_y1, lerp_y2, w);
    return result;
}

float Application::mix(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

float Application::smooth_step(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}


// Implementation of the "Fractal sum" noise function (using "perlin_noise_improved" above).
// The solution has to work for arbitrary initial frequency and amplitude as well as various input frequency or amplitude multipliers.
// The equation std::floor(std::log2(info.sampling_frequency) - 1) from slides will not work for arbitrary numbers, please use Nyquist theorem directly.
// HINTS:
// - You will need the following variables from the info:
//        info.frequency_multiplier -> a noise frequency-change coefficient (by default 2)
//        info.amplitude_multiplier -> an amplitude-change coefficient (by default 0.5)
//        info.sampling_frequency   -> number of texels between two neighbour lattice points (by default 64)
//        info.noise_frequency,     -> initial frequency for the composed octaves (by default 1)
//        info.start_amplitude      -> initial amplitude for the composed octaves (by default 1)
float Application::fractalsum(glm::vec3 const &pos, SpectralSystesisInfo const &info) {
    float total = 0.0f;
    float frequency = info.noise_frequency;
    float amplitude = info.start_amplitude;

    float maxFrequency = 0.5f * info.sampling_frequency;

    while (frequency <= maxFrequency) {
        glm::vec3 posScaled = pos * frequency;

        total += perlin_noise_improved(posScaled) * amplitude;

        frequency *= info.frequency_multiplier;
        amplitude *= info.amplitude_multiplier;
    }

    return total;
}


// Implementation of the "Turbulence" noise function (using "perlin_noise_improved" above).
// The solution has to work for arbitrary initial frequency and amplitude as well as various input frequency or amplitude multipliers.
// The equation std::floor(std::log2(info.sampling_frequency) - 1) from slides will not work for arbitrary numbers, please use Nyquist theorem directly.
// HINTS:
// - You will need the following variables from the info:
//        info.frequency_multiplier -> a noise frequency-change coefficient (by default 2)
//        info.amplitude_multiplier -> an amplitude-change coefficient (by default 0.5)
//        info.sampling_frequency   -> number of texels between two neighbour lattice points (by default 64)
//        info.noise_frequency,     -> initial frequency for the composed octaves (by default 1)
//        info.start_amplitude      -> initial amplitude for the composed octaves (by default 1)
// - std::fabs(x) or std::fabsf(x) -> These functions accepts double or float number "x" and returns its absolute value.
// - WARNING std::abs(x) requires integer and will thus not work correctly.
float Application::turbulence(glm::vec3 const &pos, SpectralSystesisInfo const &info) {
    float total = 0.0f;
    float frequency = info.noise_frequency;
    float amplitude = info.start_amplitude;

    float maxFrequency = 0.5f * info.sampling_frequency;

    while (frequency <= maxFrequency) {
        glm::vec3 posScaled = pos * frequency;

        // Same as fractal sum, but with ABSOLUTE VALUE HERE
        total += std::fabs(perlin_noise_improved(posScaled)) * amplitude;

        frequency *= info.frequency_multiplier;
        amplitude *= info.amplitude_multiplier;
    }

    return total;
}


// Implementation of the "Marble" noise function (using "perlin_noise_improved" above).
// The solution has to work for arbitrary initial frequency and amplitude as well as various input frequency or amplitude multipliers.
// The equation std::floor(std::log2(info.sampling_frequency) - 1) from slides will not work for arbitrary numbers, please use Nyquist theorem directly.
// HINTS:
// - You will need the following variables from the info:
//        info.frequency_multiplier -> a noise frequency-change coefficient (by default 2)
//        info.amplitude_multiplier -> an amplitude-change coefficient (by default 0.5)
//        info.sampling_frequency   -> number of texels between two neighbour lattice points (by default 64)
//        info.noise_frequency,     -> initial frequency for the composed octaves (by default 1)
//        info.start_amplitude      -> initial amplitude for the composed octaves (by default 1)
//        info.arg0                 -> the alpha value from slides (by default 10)
// - std::sin(x) or std:sinf(x) -> These functions accepts double or float number "x" representing an angle in radians and returns its sinus.
float Application::marble(glm::vec3 const &pos, SpectralSystesisInfo const &info) {
    // In the slides, the marble formula contains the fractal sum formula, so I'll just use that.
    float fs = fractalsum(pos, info);
    float marble_value = std::sin(info.arg0 * (pos.x + fs));
    return marble_value;
}

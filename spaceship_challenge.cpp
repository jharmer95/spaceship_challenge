// Submission by Jackson Harmer

// The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.
// You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission. See Other Information below.

#if __cplusplus < 201709
#    error C++20 must be enabled
#endif

#if __GNUC__ < 10
#    error Only GCC 10+ is supported for the C++20 features here
#endif

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

class Spaceship
{
public:
    ~Spaceship() = default;

    // Deleting default constructor
    Spaceship() = delete;

    // Using constructor instead of static function
    // Making constructor explicit and noexcept
    explicit Spaceship(std::vector<std::string>&& part_list) noexcept;

    // Spaceship can be copy/move constructed and assigned
    Spaceship(const Spaceship& other) = default;
    Spaceship(Spaceship&& other) = default;
    Spaceship& operator=(const Spaceship& other) = default;
    Spaceship& operator=(Spaceship&& other) = default;

    // Spaceship for the Spaceship 🙂
    auto operator<=>(const Spaceship& other) const noexcept = default;

    // Print is a bit prettier
    void Print() const noexcept
    {
        try
        {
            // This would be a great place for std::format
            std::cout << "\nThis ship is loaded with:"
                      << "\n  Engine: " << _parts.at(Part_Type::Engine)
                      << "\n  Fuselage: " << _parts.at(Part_Type::Fuselage)
                      << "\n  Cabin: " << _parts.at(Part_Type::Cabin)
                      << "\n  Armor: " << _parts.at(Part_Type::Armor)
                      << "\n  Wings:\n    (small): " << _smallWings
                      << "\n    (large): " << _largeWings;

            std::cout << "\n  Weapons: [";

            // Using C++20 ranges
            for (const auto& weapon : std::views::all(_weapons)
                    | std::views::take(_weapons.size() - 1))
            {
                std::cout << weapon << ", ";
            }

            std::cout << _weapons.back() << "]\n";
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Exception: \"" << ex.what() << "\"\n";
        }
    }

private:
    // Using hashmap for improved performance
    enum class Part_Type : uint8_t
    {
        Engine,
        Fuselage,
        Cabin,
        Wings,
        Armor,
        Weapon
    };

    std::unordered_map<Part_Type, std::string> _parts{};

    std::string _smallWings{};
    std::string _largeWings{};

    // Utilizing std::array for algorithm support
    std::array<std::string, 4> _weapons{};
};

Spaceship::Spaceship(std::vector<std::string>&& part_list) noexcept
{
    // Map of types to corresponding strings, would be a great place for 'using enum'
    const std::unordered_map<Part_Type, const char*> part_types_list{
        { Part_Type::Engine, "engine" }, { Part_Type::Fuselage, "fuselage" },
        { Part_Type::Cabin, "cabin" }, { Part_Type::Wings, "wings" },
        { Part_Type::Armor, "armor" }, { Part_Type::Weapon, "weapon" }
    };

    std::vector<std::string> weaponParts;

    std::random_device rd;
    std::mt19937 g(rd());

    // Single shuffle vs. multiple shuffles
    std::shuffle(part_list.begin(), part_list.end(), g);

    // This nested loop keeps code DRY
    for (auto part_str : part_list)
    {
        for (const auto& type_str : part_types_list)
        {
            if (part_str.find(type_str.second) != std::string::npos)
            {
                if (type_str.first == Part_Type::Weapon)
                {
                    weaponParts.push_back(std::move(part_str));
                }
                else if (type_str.first == Part_Type::Wings)
                {
                    if (_smallWings.empty())
                    {
                        _smallWings = std::move(part_str);
                    }
                    else if (_largeWings.empty())
                    {
                        _largeWings = std::move(part_str);
                    }
                }
                else
                {
                    _parts[type_str.first] = std::move(part_str);
                }
            }
        }
    }

    // Move into weapons array
    std::copy_n(std::make_move_iterator(weaponParts.begin()),
        std::min(_weapons.size(), weaponParts.size()), _weapons.begin());
}

// Using concepts, pretty trivial example but wanted to use it
template<typename T>
concept PathType = std::constructible_from<std::filesystem::path, T>;

int main(const int argc, const char* const argv[]) noexcept
{
    try
    {
        // parts_list parsing is lambda
        // taking advantage of C++20 templated lambdas and concepts
        const auto fetch_parts_list = []<PathType T>(T&& fname) {
            // Using std::filesystem
            if (!std::filesystem::exists(fname))
            {
                // Would be nice to have std::format here
                std::stringstream err_mesg;
                err_mesg << "file: '" << fname << "' does not exist!";
                throw std::runtime_error(err_mesg.str());
            }

            std::ifstream file(fname);

            // Early exit
            if (!file.is_open())
            {
                // Would be nice to have std::format here
                std::stringstream err_mesg;
                err_mesg << "file: '" << fname << "' could not be opened!";
                throw std::runtime_error(err_mesg.str());
            }

            std::vector<std::string> vec;

            // Reserving the size for the vector (not really useful for such a
            // small input text, but it will scale)
            const auto line_count =
                std::count(std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>(), '\n');

            file.seekg(0, std::ios::beg);
            vec.reserve(static_cast<unsigned>(line_count));

            std::string line;

            while (std::getline(file, line))
            {
                // Moving line back
                vec.push_back(std::move(line));
            }

            std::cout << "Parts loaded from: " << fname << '\n';
            return vec;
        };

        // Ternary for short-circuiting
        const auto parts_filename = argc > 1 ? argv[1] : "vehicle_parts.txt";

        // Only printing once so use r-value
        Spaceship{ fetch_parts_list(parts_filename) }.Print();
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: \"" << ex.what() << "\"\n";
        return 1;
    }
}

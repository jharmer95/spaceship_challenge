#include <algorithm>
#include <array>
#include <compare>
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
    auto operator<=>(const Spaceship& other) const = default;

    // Print is a bit prettier
    void Print() const noexcept
    {
        std::cout << "\nThis ship is loaded with:\n  Engine: "
                  << _parts.at(Part_Type::Engine)
                  << "\n  Fuselage: " << _parts.at(Part_Type::Fuselage)
                  << "\n  Cabin: " << _parts.at(Part_Type::Cabin)
                  << "\n  Wings: " << _parts.at(Part_Type::Wings)
                  << "\n  Armor: " << _parts.at(Part_Type::Armor);

        std::cout << "\n  Weapons: [";

#if __GNUC__ >= 10
        // Using C++20 ranges
        for (const auto& weapon :
            std::views::all(_weapons) | std::views::take(_weapons.size() - 1))
        {
            std::cout << weapon << ", ";
        }

        std::cout << _weapons.back() << "]\n";
#else
        auto it = _weapons.begin();

        while (true)
        {
            if (!it->empty())
            {
                std::cout << *it;

                if (++it == _weapons.end())
                {
                    break;
                }

                std::cout << ", ";
            }
        }

        std::cout << "]\n";
#endif // __GNUC__ >= 10
    }

private:
    // Using hashmap for improved performance
    enum class Part_Type : uint8_t
    {
        Engine,
        Fuselage,
        Cabin,
        Wings,
        Armor
    };

    std::unordered_map<Part_Type, std::string> _parts;

    // Utilizing std::array for algorithm support
    std::array<std::string, 4> _weapons;
};

Spaceship::Spaceship(std::vector<std::string>&& part_list) noexcept
{
    // Map of types to corresponding strings
    const std::unordered_map<Part_Type, std::string_view> part_types_list{
        { Part_Type::Engine, "engine" }, { Part_Type::Fuselage, "fuselage" },
        { Part_Type::Cabin, "cabin" }, { Part_Type::Wings, "wings" },
        { Part_Type::Armor, "armor" }
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
            if (part_str.find("weapon") != std::string::npos)
            {
                weaponParts.push_back(std::move(part_str));
            }
            else if (part_str.find(type_str.second) != std::string::npos)
            {
                _parts[type_str.first] = std::move(part_str);
            }
        }
    }

    // Move into weapons array
    std::copy_n(std::make_move_iterator(weaponParts.begin()),
        std::min(_weapons.size(), weaponParts.size()), _weapons.begin());
}

int main(const int argc, const char* const argv[])
{
    try
    {
        // parts_list parsing is lambda
        const auto fetch_parts_list = [](std::string&& fname) {
            // Using std::filesystem
            if (!std::filesystem::exists(fname))
            {
                throw std::runtime_error(
                    "file: '" + fname + "' does not exist!");
            }

            std::ifstream file(fname);

            // Early exit
            if (!file.is_open())
            {
                throw std::runtime_error(
                    "file: '" + fname + "' could not be opened!");
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
        const char* const parts_filename =
            argc > 1 ? argv[1] : "vehicle_parts.txt";

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

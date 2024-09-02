#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>
using namespace std;

// Represents a bounding box or spatial region
struct Rectangle {
    double x_min, y_min, x_max, y_max;

    Rectangle(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0)
        : x_min(x1), y_min(y1), x_max(x2), y_max(y2) {}

    bool intersects(const Rectangle& other) const {
        return !(x_min > other.x_max || x_max < other.x_min || y_min > other.y_max || y_max < other.y_min);
    }
};

// Represents a property with its details and bounding box
class Property {
public:
    std::string location;
    double price;
    double area;
    int bedrooms;
    Rectangle bbox;

    Property(const std::string& loc, double p, double a, int b, const Rectangle& box)
        : location(loc), price(p), area(a), bedrooms(b), bbox(box) {}
};

// Represents an R-tree node which can either be an internal node or a leaf node
class RTreeNode {
public:
    std::vector<RTreeNode*> children;  // For internal nodes, this holds other nodes or properties
    std::vector<Property*> leaf_properties; // For leaf nodes, this holds properties
    Rectangle bounding_box;
    bool is_leaf;

    RTreeNode(Rectangle bbox, bool leaf = false)
        : bounding_box(bbox), is_leaf(leaf) {}

    // Insert a property into the node
    void insert(Property* child) {
        if (is_leaf) {
            leaf_properties.push_back(child);
        } else {
            children.push_back(new RTreeNode(child->bbox)); // Create a new internal node to hold this property
            updateBoundingBox();
        }
    }

    // Update the bounding box of this node based on its children
    void updateBoundingBox() {
        if (children.empty() && leaf_properties.empty()) return;
        double x_min = bounding_box.x_min;
        double y_min = bounding_box.y_min;
        double x_max = bounding_box.x_max;
        double y_max = bounding_box.y_max;

        if (is_leaf) {
            for (const auto& prop : leaf_properties) {
                x_min = std::min(x_min, prop->bbox.x_min);
                y_min = std::min(y_min, prop->bbox.y_min);
                x_max = std::max(x_max, prop->bbox.x_max);
                y_max = std::max(y_max, prop->bbox.y_max);
            }
        } else {
            for (const auto& node : children) {
                x_min = std::min(x_min, node->bounding_box.x_min);
                y_min = std::min(y_min, node->bounding_box.y_min);
                x_max = std::max(x_max, node->bounding_box.x_max);
                y_max = std::max(y_max, node->bounding_box.y_max);
            }
        }

        bounding_box = Rectangle(x_min, y_min, x_max, y_max);
    }
};

// Represents the R-tree structure
class RTree {
    RTreeNode* root;

public:
    RTree() {
        root = new RTreeNode(Rectangle(0, 0, 100, 100), true); // Define an initial bounding box
    }

    // Insert a property into the R-tree
    void insert(Property* prop) {
        root->insert(prop);
    }

    // Query properties within a specified range
    std::vector<Property*> query(Rectangle range) {
        std::vector<Property*> results;
        queryRecursive(root, range, results);
        return results;
    }

    // Query properties near a specified location and within a distance range
    std::vector<Property*> queryNearLocation(double x, double y, double distance_km, double max_price, double min_area, int min_bedrooms) {
        std::vector<Property*> results;
        Rectangle search_area(x - distance_km, y - distance_km, x + distance_km, y + distance_km);
        std::vector<Property*> properties = query(search_area);

        for (const auto& prop : properties) {
            double dist = calculateDistance(x, y, (prop->bbox.x_min + prop->bbox.x_max) / 2, (prop->bbox.y_min + prop->bbox.y_max) / 2);
            // cout<<"dist is"<<dist<<endl;
            if (dist <= distance_km &&
                prop->price <= max_price &&
                prop->area >= min_area &&
                prop->bedrooms >= min_bedrooms) {
                results.push_back(prop);
            }
        }

        return results;
    }

private:
    // Recursive function to perform the query
    void queryRecursive(RTreeNode* node, Rectangle range, std::vector<Property*>& results) {
        if (!node->bounding_box.intersects(range)) return;

        if (node->is_leaf) {
            for (const auto& prop : node->leaf_properties) {
                if (range.intersects(prop->bbox)) {
                    results.push_back(prop);
                }
            }
        } else {
            for (const auto& child : node->children) {
                queryRecursive(child, range, results);
            }
        }
    }

    // Calculate the Euclidean distance between two points (latitude and longitude) in kilometers
    double calculateDistance(double x1, double y1, double x2, double y2) {
        int distance= sqrt(pow((x2-x1),2) + pow((y2-y1),2));
        return distance;
    }
};

// Clear input buffer function
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    RTree tree;
    int choice;

    do {
        std::cout << "\nReal Estate Property System\n";
        std::cout << "1. Insert Property\n2. Query Properties\n3. Query Near Location\n4. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        clearInputBuffer(); // Clear any leftover newline characters

        if (choice == 1) {
            std::string location;
            double price, area;
            int bedrooms;
            double x_min, y_min, x_max, y_max;

            std::cout << "Enter property location: ";
            std::getline(std::cin, location);

            std::cout << "Enter property price: ";
            while (!(std::cin >> price) || price < 0) {
                std::cout << "Invalid input. Please enter a positive number for price: ";
                clearInputBuffer();
            }

            std::cout << "Enter property area: ";
            while (!(std::cin >> area) || area < 0) {
                std::cout << "Invalid input. Please enter a positive number for area: ";
                clearInputBuffer();
            }

            std::cout << "Enter number of bedrooms: ";
            while (!(std::cin >> bedrooms) || bedrooms < 0) {
                std::cout << "Invalid input. Please enter a non-negative integer for bedrooms: ";
                clearInputBuffer();
            }

            std::cout << "Enter property bounding box (x_min y_min x_max y_max): ";
            while (!(std::cin >> x_min >> y_min >> x_max >> y_max) || x_min > x_max || y_min > y_max) {
                std::cout << "Invalid input. Ensure x_min <= x_max and y_min <= y_max. Enter bounding box (x_min y_min x_max y_max): ";
                clearInputBuffer();
            }

            Rectangle bbox(x_min, y_min, x_max, y_max);
            Property* prop = new Property(location, price, area, bedrooms, bbox);
            tree.insert(prop);
            std::cout << "Property inserted.\n";

        } else if (choice == 2) {
            double q_x_min, q_y_min, q_x_max, q_y_max;
            std::cout << "Enter query range (x_min y_min x_max y_max): ";
            while (!(std::cin >> q_x_min >> q_y_min >> q_x_max >> q_y_max) || q_x_min > q_x_max || q_y_min > q_y_max) {
                std::cout << "Invalid input. Ensure x_min <= x_max and y_min <= y_max. Enter query range (x_min y_min x_max y_max): ";
                clearInputBuffer();
            }
            Rectangle query_range(q_x_min, q_y_min, q_x_max, q_y_max);

            auto results = tree.query(query_range);
            std::cout << "Query results:\n";
            if (results.empty()) {
                std::cout << "No properties found within the specified range.\n";
            } else {
                for (const auto& prop : results) {
                    std::cout << "Location: " << prop->location
                              << ", Price: $" << prop->price
                              << ", Area: " << prop->area << " sq. ft."
                              << ", Bedrooms: " << prop->bedrooms
                              << ", Bounding Box: (" << prop->bbox.x_min << ", " << prop->bbox.y_min
                              << ", " << prop->bbox.x_max << ", " << prop->bbox.y_max << ")\n";
                }
            }

        } else if (choice == 3) {
            double user_x, user_y, distance_km, max_price, min_area;
            int min_bedrooms;

            std::cout << "Enter your location (x y): ";
            while (!(std::cin >> user_x >> user_y)) {
                std::cout << "Invalid input. Enter your location (x y): ";
                clearInputBuffer();
            }

            std::cout << "Enter search distance (km): ";
            while (!(std::cin >> distance_km) || distance_km < 0) {
                std::cout << "Invalid input. Please enter a non-negative number for distance: ";
                clearInputBuffer();
            }

            std::cout << "Enter maximum price: ";
            while (!(std::cin >> max_price) || max_price < 0) {
                std::cout << "Invalid input. Please enter a non-negative number for price: ";
                clearInputBuffer();
            }

            std::cout << "Enter minimum area: ";
            while (!(std::cin >> min_area) || min_area < 0) {
                std::cout << "Invalid input. Please enter a non-negative number for area: ";
                clearInputBuffer();
            }

            std::cout << "Enter minimum number of bedrooms: ";
            while (!(std::cin >> min_bedrooms) || min_bedrooms < 0) {
                std::cout << "Invalid input. Please enter a non-negative integer for bedrooms: ";
                clearInputBuffer();
            }

            auto results = tree.queryNearLocation(user_x, user_y, distance_km, max_price, min_area, min_bedrooms);
            std::cout << "Query results:\n";
            if (results.empty()) {
                std::cout << "No properties found within the specified criteria.\n";
            } else {
                for (const auto& prop : results) {
                    std::cout << "Location: " << prop->location
                              << ", Price: $" << prop->price
                              << ", Area: " << prop->area << " sq. ft."
                              << ", Bedrooms: " << prop->bedrooms
                              << ", Bounding Box: (" << prop->bbox.x_min << ", " << prop->bbox.y_min
                              << ", " << prop->bbox.x_max << ", " << prop->bbox.y_max << ")\n";
                }
            }

        } else if (choice == 4) {
            std::cout << "Exiting...\n";
        } else {
            std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 4);

    return 0;
}
#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <iostream>
#include <string>
#include <fstream>
#include <functional>

struct Event {
    int id;
    std::string name;
    std::string date;
    std::string startTime;
    std::string endTime;

    Event() = default;

    Event(int i, const std::string& n, const std::string& d, const std::string& s, const std::string& e)
        : id(i), name(n), date(d), startTime(s), endTime(e) {}

    bool operator<(const Event& other) const {
        if (date != other.date) return date < other.date;
        return startTime < other.startTime;
    }

    bool operator>(const Event& other) const {
        if (date != other.date) return date > other.date;
        return startTime > other.startTime;
    }

    friend std::ostream& operator<<(std::ostream& os, const Event& event) {
        os << event.id << " " << event.name << " " << event.date << " " << event.startTime << " " << event.endTime;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Event& event) {
        is >> event.id >> event.name >> event.date >> event.startTime >> event.endTime;
        return is;
    }
};

struct AVLNode {
    Event event;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Event e) : event(e), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int balanceFactor(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    void updateHeight(AVLNode* node) {
        if (node) {
            node->height = 1 + std::max(height(node->left), height(node->right));
        }
    }

    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    AVLNode* balance(AVLNode* node) {
        if (!node) return node;

        updateHeight(node);

        int balance = balanceFactor(node);

        if (balance > 1 && balanceFactor(node->left) >= 0) {
            return rotateRight(node);
        }

        if (balance > 1 && balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 && balanceFactor(node->right) <= 0) {
            return rotateLeft(node);
        }

        if (balance < -1 && balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    AVLNode* insert(AVLNode* node, Event event) {
        if (!node) return new AVLNode(event);

        if (event < node->event) {
            node->left = insert(node->left, event);
        } else if (event > node->event) {
            node->right = insert(node->right, event);
        } else {
            return node; // Duplicate events are not allowed
        }

        return balance(node);
    }

    void inorder(AVLNode* node, std::ostream& os) const {
        if (node) {
            inorder(node->left, os);
            os << node->event << std::endl;
            inorder(node->right, os);
        }
    }

    AVLNode* findMin(AVLNode* node) {
        while (node->left != nullptr) node = node->left;
        return node;
    }

    AVLNode* remove(AVLNode* root, Event event) {
        if (!root) return root;

        if (event < root->event) {
            root->left = remove(root->left, event);
        } else if (event > root->event) {
            root->right = remove(root->right, event);
        } else {
            if (!root->left) {
                AVLNode* temp = root->right;
                delete root;
                return temp;
            } else if (!root->right) {
                AVLNode* temp = root->left;
                delete root;
                return temp;
            }

            AVLNode* temp = findMin(root->right);
            root->event = temp->event;
            root->right = remove(root->right, temp->event);
        }

        return balance(root);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(Event event) {
        root = insert(root, event);
    }

    void remove(Event event) {
        root = remove(root, event);
    }

    void inorder(std::ostream& os) const {
        inorder(root, os);
    }

    bool detectConflicts(const Event& newEvent) const {
        return detectConflicts(root, newEvent);
    }

    bool detectConflicts(AVLNode* node, const Event& newEvent) const {
        if (!node) return false;

        if (!(newEvent.endTime <= node->event.startTime || newEvent.startTime >= node->event.endTime)) {
            return true;
        }

        return detectConflicts(node->left, newEvent) || detectConflicts(node->right, newEvent);
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            return;
        }

        Event event;
        while (infile >> event) {
            insert(event);
        }
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream outfile(filename);
        inorder(outfile);
    }
};

#endif // AVL_TREE_H

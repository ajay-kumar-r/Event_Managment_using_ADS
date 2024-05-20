#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <iostream>
#include <string>
#include <fstream>
#include <functional>

using namespace std;

struct Event {
    int id;
    string name;
    string date;
    string startTime;
    string endTime;

    Event() : id(0), name(""), date(""), startTime(""), endTime("") {}
    Event(int i, string n, string d, string st, string et) : id(i), name(n), date(d), startTime(st), endTime(et) {}

    bool operator<(const Event& other) const {
        if (date == other.date) {
            return startTime < other.startTime;
        }
        return date < other.date;
    }

    bool operator>(const Event& other) const {
        if (date == other.date) {
            return startTime > other.startTime;
        }
        return date > other.date;
    }

    bool operator==(const Event& other) const {
        return date == other.date && startTime == other.startTime && endTime == other.endTime;
    }

    friend ostream& operator<<(ostream& os, const Event& e) {
        os << e.id << ": " << e.name << " (" << e.date << " " << e.startTime << "-" << e.endTime << ")";
        return os;
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
            node->height = 1 + max(height(node->left), height(node->right));
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

    void inorder(AVLNode* node, ostream& os) const {
        if (node) {
            inorder(node->left, os);
            os << node->event << endl;
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

    bool detectConflicts(AVLNode* node, const Event& newEvent) {
        if (!node) return false;

        if (!(newEvent.endTime <= node->event.startTime || newEvent.startTime >= node->event.endTime)) {
            return true;
        }

        if (newEvent < node->event) {
            return detectConflicts(node->left, newEvent);
        } else {
            return detectConflicts(node->right, newEvent);
        }
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(Event event) {
        root = insert(root, event);
    }

    void remove(Event event) {
        root = remove(root, event);
    }

    bool detectConflicts(const Event& event) {
        return detectConflicts(root, event);
    }

    void visualize(const string& filename) const {
        ofstream outfile(filename);
        outfile << "digraph AVLTree {\n";
        outfile << "node [fontname=\"Arial\"];\n";

        function<void(AVLNode*, ofstream&)> writeNode = [&](AVLNode* node, ofstream& out) {
            if (!node) return;
            out << "\"" << node->event.name << "\";\n";
            if (node->left) {
                out << "\"" << node->event.name << "\" -> \"" << node->left->event.name << "\";\n";
                writeNode(node->left, out);
            }
            if (node->right) {
                out << "\"" << node->event.name << "\" -> \"" << node->right->event.name << "\";\n";
                writeNode(node->right, out);
            }
        };

        writeNode(root, outfile);

        outfile << "}\n";
    }

    void inorder(ostream& os) const {
        inorder(root, os);
    }
};

#endif


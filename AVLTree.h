#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include <ncurses.h>

using namespace std;

class Event {
public:
    int id;
    string name;
    string date;
    string startTime;
    string endTime;
    set<int> dependencies;

    Event(int id = 0, string name = "", string date = "", string startTime = "", string endTime = "")
        : id(id), name(name), date(date), startTime(startTime), endTime(endTime) {}

    bool operator<(const Event& other) const {
        if (date != other.date) return date < other.date;
        if (startTime != other.startTime) return startTime < other.startTime;
        return endTime < other.endTime;
    }

    bool operator>(const Event& other) const {
        return other < *this;
    }

    bool operator==(const Event& other) const {
        return id == other.id;
    }

    bool operator!=(const Event& other) const {
        return !(*this == other);
    }

    friend ostream& operator<<(ostream& os, const Event& event);
    friend istream& operator>>(istream& is, Event& event);
};

ostream& operator<<(ostream& os, const Event& event) {
    os << event.id << "," << event.name << "," << event.date << "," << event.startTime << "," << event.endTime;
        for (int dep : event.dependencies) {
            os << "," << dep;
        }
        return os;
}

istream& operator>>(istream& is, Event& event) {
    string line;
        if (getline(is, line)) {
            stringstream ss(line);
            string token;
            getline(ss, token, ',');
            event.id = stoi(token);
            getline(ss, event.name, ',');
            getline(ss, event.date, ',');
            getline(ss, event.startTime, ',');
            getline(ss, event.endTime, ',');

            while (getline(ss, token, ',')) {
                event.dependencies.insert(stoi(token));
            }
        }
        return is;
}

struct AVLNode {
    Event event;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const Event& event, AVLNode* lt, AVLNode* rt, int h = 0)
        : event(event), left(lt), right(rt), height(h) {}
};

class AVLTree {
public:
    AVLTree() : root(nullptr) {}

    ~AVLTree() {
        makeEmpty(root);
    }

    void insert(const Event& event) {
        insert(event, root);
    }

    void remove(int id) {
        remove(id, root);
    }

    bool detectConflicts(const Event& event) const {
        return detectConflicts(event, root);
    }

    void visualize() const {
        ofstream outfile("avltree.dot");
        if (!outfile) {
            cerr << "Error creating dot file" << endl;
            return;
        }
        outfile << "digraph AVLTree {\n";
        if (root) {
            visualize(outfile, root);
        }
        outfile << "}\n";
        outfile.close();
        system("dot -Tpng avltree.dot -o avltree.png");
        system("xdg-open avltree.png");
    }

private:
    AVLNode* root;

    void insert(const Event& event, AVLNode*& t) {
        if (t == nullptr) {
            t = new AVLNode(event, nullptr, nullptr);
        } else if (event < t->event) {
            insert(event, t->left);
            if (height(t->left) - height(t->right) == 2) {
                if (event < t->left->event) {
                    rotateWithLeftChild(t);
                } else {
                    doubleWithLeftChild(t);
                }
            }
        } else if (event > t->event) {
            insert(event, t->right);
            if (height(t->right) - height(t->left) == 2) {
                if (event > t->right->event) {
                    rotateWithRightChild(t);
                } else {
                    doubleWithRightChild(t);
                }
            }
        }
        t->height = max(height(t->left), height(t->right)) + 1;
    }

    void remove(int id, AVLNode*& t) {
        if (t == nullptr) {
            return;
        }
        if (id < t->event.id) {
            remove(id, t->left);
        } else if (id > t->event.id) {
            remove(id, t->right);
        } else if (t->left != nullptr && t->right != nullptr) {
            t->event = findMin(t->right)->event;
            remove(t->event.id, t->right);
        } else {
            AVLNode* oldNode = t;
            t = (t->left != nullptr) ? t->left : t->right;
            delete oldNode;
        }
        balance(t);
    }

    bool detectConflicts(const Event& event, AVLNode* t) const {
        if (t == nullptr) {
            return false;
        }
        if (event.date == t->event.date && 
            !(event.endTime <= t->event.startTime || event.startTime >= t->event.endTime)) {
            return true;
        }
        return detectConflicts(event, t->left) || detectConflicts(event, t->right);
    }
    void visualize(ofstream& outfile, AVLNode* t) const {
        if (t->left) {
            outfile << "\"" << t->event.name << "\\n" << t->event.date << "\\n" << t->event.startTime << "-" << t->event.endTime << "\" -> \"" << t->left->event.name << "\\n" << t->left->event.date << "\\n" << t->left->event.startTime << "-" << t->left->event.endTime << "\";\n";
            visualize(outfile, t->left);
        }
        if (t->right) {
            outfile << "\"" << t->event.name << "\\n" << t->event.date << "\\n" << t->event.startTime << "-" << t->event.endTime << "\" -> \"" << t->right->event.name << "\\n" << t->right->event.date << "\\n" << t->right->event.startTime << "-" << t->right->event.endTime << "\";\n";
            visualize(outfile, t->right);
        }
    }

    int height(AVLNode* t) const {
        return t == nullptr ? -1 : t->height;
    }

    void rotateWithLeftChild(AVLNode*& k2) {
        AVLNode* k1 = k2->left;
        k2->left = k1->right;
        k1->right = k2;
        k2->height = max(height(k2->left), height(k2->right)) + 1;
        k1->height = max(height(k1->left), k2->height) + 1;
        k2 = k1;
    }

    void rotateWithRightChild(AVLNode*& k1) {
        AVLNode* k2 = k1->right;
        k1->right = k2->left;
        k2->left = k1;
        k1->height = max(height(k1->left), height(k1->right)) + 1;
        k2->height = max(height(k2->right), k1->height) + 1;
        k1 = k2;
    }

    void doubleWithLeftChild(AVLNode*& k3) {
        rotateWithRightChild(k3->left);
        rotateWithLeftChild(k3);
    }

    void doubleWithRightChild(AVLNode*& k1) {
        rotateWithLeftChild(k1->right);
        rotateWithRightChild(k1);
    }

    AVLNode* findMin(AVLNode* t) const {
        if (t == nullptr) {
            return nullptr;
        }
        if (t->left == nullptr) {
            return t;
        }
        return findMin(t->left);
    }

    void balance(AVLNode*& t) {
        if (t == nullptr) {
            return;
        }
        if (height(t->left) - height(t->right) > 1) {
            if (height(t->left->left) >= height(t->left->right)) {
                rotateWithLeftChild(t);
            } else {
                doubleWithLeftChild(t);
            }
        } else if (height(t->right) - height(t->left) > 1) {
            if (height(t->right->right) >= height(t->right->left)) {
                rotateWithRightChild(t);
            } else {
                doubleWithRightChild(t);
            }
        }
        t->height = max(height(t->left), height(t->right)) + 1;
    }

    void makeEmpty(AVLNode*& t) {
        if (t != nullptr) {
            makeEmpty(t->left);
            makeEmpty(t->right);
            delete t;
        }
        t = nullptr;
    }
};

#endif // AVLTREE_H

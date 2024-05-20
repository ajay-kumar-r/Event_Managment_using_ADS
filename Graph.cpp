#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <vector>
#include <algorithm>
#include <functional>
#include "AVLTree.h"

using namespace std;

class EventGraph {
private:
    vector<Event> events;

public:
    void addEvent(const Event& event) {
        events.push_back(event);
    }

    void updateEvent(int id, const string& newName, const string& newDate, const string& newStartTime, const string& newEndTime) {
        for (auto& event : events) {
            if (event.id == id) {
                event.name = newName;
                event.date = newDate;
                event.startTime = newStartTime;
                event.endTime = newEndTime;
                break;
            }
        }
    }

    void exportGraph(const string& filename) const {
        ofstream outfile(filename);
        outfile << "digraph EventGraph {\n";
        outfile << "node [fontname=\"Arial\"];\n";

        for (const auto& event : events) {
            outfile << "\"" << event.name << "\" [label=\"" << event.name << "\\n" << event.date << "\\n" << event.startTime << "-" << event.endTime << "\"];\n";
        }

        outfile << "}\n";
    }

    void viewSchedule() const {
        clear();
        mvprintw(0, 0, "Event Schedule:");
        int row = 1;
        for (const auto& event : events) {
            mvprintw(row++, 0, "%d: %s (%s %s-%s)", event.id, event.name.c_str(), event.date.c_str(), event.startTime.c_str(), event.endTime.c_str());
        }
        refresh();
        mvprintw(row, 0, "Press any key to return to the main menu...");
        getch();
    }

    bool hasConflict(const Event& newEvent) const {
        for (const auto& event : events) {
            if (!(newEvent.endTime <= event.startTime || newEvent.startTime >= event.endTime)) {
                return true;
            }
        }
        return false;
    }
};

// Initialize Ncurses
void initialize_ncurses() {
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_GREEN, COLOR_BLACK);
}

// Draw a box with given dimensions and color
void draw_box(int starty, int startx, int height, int width, int color_pair) {
    attron(COLOR_PAIR(color_pair));
    for (int i = startx; i < startx + width; ++i) {
        mvprintw(starty, i, "-");
        mvprintw(starty + height, i, "-");
    }
    for (int i = starty; i < starty + height; ++i) {
        mvprintw(i, startx, "|");
        mvprintw(i, startx + width, "|");
    }
    mvprintw(starty, startx, "+");
    mvprintw(starty + height, startx, "+");
    mvprintw(starty, startx + width, "+");
    mvprintw(starty + height, startx + width, "+");
    attroff(COLOR_PAIR(color_pair));
}

// Display Menu with enhanced UI
void display_menu() {
    clear();
    int starty = (LINES - 15) / 2;
    int startx = (COLS - 50) / 2;
    draw_box(starty, startx, 12, 40, 2);

    attron(COLOR_PAIR(3));
    mvprintw(starty + 1, startx + 15, "Event Scheduler");
    attroff(COLOR_PAIR(3));

    mvprintw(starty + 3, startx + 5, "1. Create Event");
    mvprintw(starty + 4, startx + 5, "2. Update Event");
    mvprintw(starty + 5, startx + 5, "3. View Schedule");
    mvprintw(starty + 6, startx + 5, "4. Detect Conflicts");
    mvprintw(starty + 7, startx + 5, "5. Visualize Event Graph");
    mvprintw(starty + 8, startx + 5, "6. Visualize AVL Tree");
    mvprintw(starty + 9, startx + 5, "7. Exit");

    refresh();
}

void create_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    mvprintw(0, 0, "Enter Event ID: ");
    int id;
    scanw("%d", &id);
    mvprintw(1, 0, "Enter Event Name: ");
    char name[50];
    getstr(name);
    mvprintw(2, 0, "Enter Date (YYYY-MM-DD): ");
    char date[11];
    getstr(date);
    mvprintw(3, 0, "Enter Start Time (HH:MM): ");
    char startTime[6];
    getstr(startTime);
    mvprintw(4, 0, "Enter End Time (HH:MM): ");
    char endTime[6];
    getstr(endTime);

    Event newEvent(id, name, date, startTime, endTime);

    if (avlTree.detectConflicts(newEvent)) {
        mvprintw(6, 0, "Conflict detected! Event not added.");
    } else {
        avlTree.insert(newEvent);
        graph.addEvent(newEvent);
        mvprintw(6, 0, "Event created successfully!");
    }

    mvprintw(7, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void update_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    mvprintw(0, 0, "Enter Event ID to Update: ");
    int id;
    scanw("%d", &id);

    mvprintw(1, 0, "Enter New Event Name: ");
    char newName[50];
    getstr(newName);
    mvprintw(2, 0, "Enter New Date (YYYY-MM-DD): ");
    char newDate[11];
    getstr(newDate);
    mvprintw(3, 0, "Enter New Start Time (HH:MM): ");
    char newStartTime[6];
    getstr(newStartTime);
    mvprintw(4, 0, "Enter New End Time (HH:MM): ");
    char newEndTime[6];
    getstr(newEndTime);

    graph.updateEvent(id, newName, newDate, newStartTime, newEndTime);
    mvprintw(6, 0, "Event updated successfully!");

    mvprintw(7, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void view_schedule(EventGraph& graph) {
    graph.viewSchedule();
}

void detect_conflicts(EventGraph& graph) {
    clear();
    mvprintw(0, 0, "Detect Conflicts functionality is integrated into event creation.");
    mvprintw(1, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void visualize_event_graph(const EventGraph& graph) {
    graph.exportGraph("event_graph.dot");
    system("dot -Tpng event_graph.dot -o event_graph.png");
    system("xdg-open event_graph.png");
}

void visualize_avl_tree(AVLTree& tree) {
    tree.visualize("avl_tree.dot");
    system("dot -Tpng avl_tree.dot -o avl_tree.png");
    system("xdg-open avl_tree.png");
}

int main() {
    AVLTree avlTree;
    EventGraph eventGraph;

    initialize_ncurses();

    while (true) {
        display_menu();
        int choice = getch() - '0';

        switch (choice) {
            case 1:
                create_event(eventGraph, avlTree);
                break;
            case 2:
                update_event(eventGraph, avlTree);
                break;
            case 3:
                view_schedule(eventGraph);
                break;
            case 4:
                detect_conflicts(eventGraph);
                break;
            case 5:
                visualize_event_graph(eventGraph);
                break;
            case 6:
                visualize_avl_tree(avlTree);
                break;
            case 7:
                endwin();
                return 0;
        }
    }

    return 0;
}


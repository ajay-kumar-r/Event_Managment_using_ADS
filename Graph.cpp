#include <iostream>
#include <fstream>
#include <ncurses.h>
#include <vector>
#include <algorithm>
#include "AVLTree.h"

int e_id = 1; // Global event ID counter

using namespace std;

class EventGraph {
private:
    vector<Event> events;

public:
    void addEvent(const Event& event) {
        events.push_back(event);
    }

    void updateEventName(int id, const string& newName) {
        for (auto& event : events) {
            if (event.id == id) {
                event.name = newName;
                break;
            }
        }
    }

    void updateEventDate(int id, const string& newDate) {
        for (auto& event : events) {
            if (event.id == id) {
                event.date = newDate;
                break;
            }
        }
    }

    void updateEventStartTime(int id, const string& newStartTime) {
        for (auto& event : events) {
            if (event.id == id) {
                event.startTime = newStartTime;
                break;
            }
        }
    }

    void updateEventEndTime(int id, const string& newEndTime) {
        for (auto& event : events) {
            if (event.id == id) {
                event.endTime = newEndTime;
                break;
            }
        }
    }

    void deleteEvent(int id) {
        events.erase(remove_if(events.begin(), events.end(), [id](const Event& event) {
            return event.id == id;
        }), events.end());
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

    void loadEvents(const string& filename) {
        ifstream infile(filename);
        if (!infile.is_open()) {
            return;
        }

        Event event;
        int maxId = 0;
        while (infile >> event) {
            addEvent(event);
            if (event.id > maxId) {
                maxId = event.id;
            }
        }
        e_id = maxId + 1; // Initialize e_id to one more than the highest ID
    }

    void saveEvents(const string& filename) const {
        ofstream outfile(filename);
        for (const auto& event : events) {
            outfile << event << endl;
        }
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
    mvprintw(starty + 1, startx + 13, "Event Scheduler");
    attroff(COLOR_PAIR(3));

    mvprintw(starty + 3, startx + 5, "1. Create Event");
    mvprintw(starty + 4, startx + 5, "2. Update Event");
    mvprintw(starty + 5, startx + 5, "3. View Schedule");
    mvprintw(starty + 6, startx + 5, "4. Detect Conflicts");
    mvprintw(starty + 7, startx + 5, "5. Visualize Event Graph");
    mvprintw(starty + 8, startx + 5, "6. Visualize AVL Tree");
    mvprintw(starty + 9, startx + 5, "7. Exit");
    mvprintw(starty + 10, startx + 5, "Enter your choice: ");

    refresh();
}

void create_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    int id = e_id++;
    mvprintw(0, 0, "Enter Event Name: ");
    char name[50];
    getstr(name);
    mvprintw(1, 0, "Enter Date (YYYY-MM-DD): ");
    char date[11];
    getstr(date);
    mvprintw(2, 0, "Enter Start Time (HH:MM): ");
    char startTime[6];
    getstr(startTime);
    mvprintw(3, 0, "Enter End Time (HH:MM): ");
    char endTime[6];
    getstr(endTime);

    Event newEvent(id, name, date, startTime, endTime);

    if (avlTree.detectConflicts(newEvent)) {
        mvprintw(5, 0, "Conflict detected! Event not added.");
        e_id--;
    } else {
        avlTree.insert(newEvent);
        graph.addEvent(newEvent);
        mvprintw(5, 0, "Event created successfully!");
        mvprintw(6, 0, "Event id: %d", id);
    }

    mvprintw(8, 0, "Press any key to return to the main menu...");
    refresh();
    getch();
}

void update_event(EventGraph& graph, AVLTree& avlTree) {
    clear();
    mvprintw(0, 0, "Enter Event ID to Update: ");
    int id;
    scanw("%d", &id);

    mvprintw(1, 0, "Select the field to update:\n");
    mvprintw(2, 0, "1. Event Name\n");
    mvprintw(3, 0, "2. Date\n");
    mvprintw(4, 0, "3. Start Time\n");
    mvprintw(5, 0, "4. End Time\n");
    mvprintw(6, 0, "Enter your choice: ");
    int choice;
    scanw("%d", &choice);

    switch (choice) {
        case 1: {
            mvprintw(8, 0, "Enter New Event Name: ");
            char newName[50];
            getstr(newName);
            graph.updateEventName(id, newName);
            break;
        }
        case 2: {
            mvprintw(8, 0, "Enter New Date (YYYY-MM-DD): ");
            char newDate[11];
            getstr(newDate);
            graph.updateEventDate(id, newDate);
            break;
        }
        case 3: {
            mvprintw(8, 0, "Enter New Start Time (HH:MM): ");
            char newStartTime[6];
            getstr(newStartTime);
            graph.updateEventStartTime(id, newStartTime);
            break;
        }
        case 4: {
            mvprintw(8, 0, "Enter New End Time (HH:MM): ");
            char newEndTime[6];
            getstr(newEndTime);
            graph.updateEventEndTime(id, newEndTime);
            break;
        }
        default:
            mvprintw(8, 0, "Invalid choice. No changes made.");
            break;
    }

    mvprintw(10, 0, "Event updated successfully! Press any key to return to the main menu...");
    refresh();
    getch();
}

int main() {
    EventGraph graph;
    AVLTree avlTree;

    graph.loadEvents("events.txt");
    avlTree.loadFromFile("avl_events.txt");

    initialize_ncurses();

    while (true) {
        display_menu();
        int choice;
        scanw("%d", &choice);

        switch (choice) {
            case 1:
                create_event(graph, avlTree);
                break;
            case 2:
                update_event(graph, avlTree);
                break;
            case 3:
                graph.viewSchedule();
                break;
            case 4: {
                // Detect Conflicts
                break;
            }
            case 5: {
                graph.exportGraph("event_graph.dot");
                mvprintw(10, 0, "Event Graph exported as event_graph.dot");
                mvprintw(11, 0, "Press any key to return to the main menu...");
                refresh();
                getch();
                break;
            }
            case 6: {
                ofstream outfile("avl_tree.txt");
                avlTree.inorder(outfile);
                outfile.close();
                mvprintw(10, 0, "AVL Tree exported as avl_tree.txt");
                mvprintw(11, 0, "Press any key to return to the main menu...");
                refresh();
                getch();
                break;
            }
            case 7:
                graph.saveEvents("events.txt");
                avlTree.saveToFile("avl_events.txt");
                endwin();
                return 0;
            default:
                mvprintw(13, 0, "Invalid choice. Please try again.");
                refresh();
                getch();
        }
    }

    return 0;
}

import networkx as nx
import matplotlib.pyplot as plt
import json

def load_event_data(filename):
    with open(filename, "r") as file:
        return json.load(file)

def create_event_graph(event_data):
    G = nx.DiGraph()
    for event, dependencies in event_data.items():
        for dependency in dependencies:
            G.add_edge(dependency, event)
    return G

event_data = load_event_data("event_data.json")
event_graph = create_event_graph(event_data)

plt.figure(figsize=(8, 6))
pos = nx.spring_layout(event_graph)
nx.draw(event_graph, pos, with_labels=True, node_color='skyblue', node_size=1500, arrowsize=20)
plt.title("Event Dependency Graph")
plt.show()

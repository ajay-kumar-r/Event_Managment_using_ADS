import json
import matplotlib.pyplot as plt
import networkx as nx

def load_avl_tree_data(filename):
    with open(filename, "r") as file:
        return json.load(file)

def parse_avl_tree(avl_data):
    G = nx.DiGraph()

    def dfs(node):
        if node is None:
            return
        G.add_node(node["id"], label=node["name"])
        if node["left"]:
            G.add_edge(node["id"], node["left"]["id"])
            dfs(node["left"])
        if node["right"]:
            G.add_edge(node["id"], node["right"]["id"])
            dfs(node["right"])

    dfs(avl_data)
    return G

avl_tree_data = load_avl_tree_data("avl_tree_data.json")
avl_graph = parse_avl_tree(avl_tree_data)

plt.figure(figsize=(8, 6))
pos = nx.spring_layout(avl_graph)
labels = nx.get_node_attributes(avl_graph, 'label')
nx.draw(avl_graph, pos, labels=labels, with_labels=True, node_color='lightgreen', node_size=1500, arrowsize=20)
plt.title("AVL Tree Visualization")
plt.show()

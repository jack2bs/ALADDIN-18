#include "base_address_init.h"

// Base address and array label initialization.
//
// For every memory access node, find the node that generated the memory
// address and assign the variable name storing the value as the current node's
// array label.

using namespace SrcTypes;

std::string BaseAddressInit::getCenteredName(size_t size) {
  return "       Init Base Address       ";
}

void BaseAddressInit::optimize() {
  EdgeNameMap edge_to_parid = get(boost::edge_name, graph);

  vertex_iter vi, vi_end;
  for (boost::tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) {
    Vertex curr_vertex = *vi;
    ExecNode* node = getNodeFromVertex(curr_vertex);
    if (boost::degree(*vi, graph) == 0) {
      if (node->get_node_id() == 637)
      {
        std::cout << "loc1\n";
      }
      continue;
    }
    // Vertex curr_vertex = *vi;
    // ExecNode* node = getNodeFromVertex(curr_vertex);
    if (!(node->is_memory_op() || node->is_gep_op())) {
      if (node->get_node_id() == 637)
      {
        std::cout << "loc2\n";
      }
      continue;
    }
    int node_microop = node->get_microop();
    // iterate its parents, until it finds the root parent
    while (true) {
      bool found_parent = false;
      in_edge_iter in_edge_it, in_edge_end;

      for (boost::tie(in_edge_it, in_edge_end) = in_edges(curr_vertex, graph);
           in_edge_it != in_edge_end;
           ++in_edge_it) {
        int edge_parid = edge_to_parid[*in_edge_it];
        /* For Load, not mem dependence. */
        /* For GEP, not the dependence that is caused by index. */
        /* For store, not the dependence caused by value or mem dependence. */
        if ((node_microop == LLVM_IR_Load && edge_parid != 1) ||
            (node_microop == LLVM_IR_GetElementPtr && edge_parid != 1) ||
            (node_microop == LLVM_IR_Store && edge_parid != 2)) {
              if (node->get_node_id() == 637)
              {
                std::cout << "loc3\n" << node->get_num_parents() << '\n';
              }
            
          continue;
            }
        unsigned parent_id = vertex_to_name[source(*in_edge_it, graph)];
        ExecNode* parent_node = exec_nodes.at(parent_id);
        int parent_microop = parent_node->get_microop();
        if (parent_microop == LLVM_IR_GetElementPtr ||
            parent_microop == LLVM_IR_Load || parent_microop == LLVM_IR_Store) {
          // remove address calculation directly
          DynamicVariable dynvar = parent_node->get_dynamic_variable();
          dynvar = call_argument_map.lookup(dynvar);
          node->set_variable(parent_node->get_variable());
          const std::string& label_pre = dynvar.get_variable()->get_name();
          std::string old = node->get_array_label();
          // std::cout << "the name here4 is: " << label_pre << " was " << old << " " << node->get_node_id() << " " << '\n';
          const std::string& label = label_pre.substr(0, label_pre.find(".sink"));
          std::cout << "the name here4 is: " << label << " was " << old << " " << node->get_node_id() << " " << '\n';
          node->set_array_label(label);
          curr_vertex = source(*in_edge_it, graph);
          node_microop = parent_microop;
          found_parent = true;
          break;
        } else if (parent_microop == LLVM_IR_Alloca) {

          node->set_variable(parent_node->get_variable());
          std::string label = exec_nodes.at(parent_id)->get_array_label();
          // std::cout << "the name here5 is: " << label << '\n';
          node->set_array_label(label);
          break;
        }
      }
      if (!found_parent)
        break;
    }
  }
}

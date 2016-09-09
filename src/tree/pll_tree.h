/*
 Copyright (C) 2016 Diego Darriba

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Diego Darriba <Diego.Darriba@h-its.org>,
 Exelixis Lab, Heidelberg Instutute for Theoretical Studies
 Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
 */
#ifndef PLL_TREE_H_
#define PLL_TREE_H_

#ifndef PLL_H_
#define PLL_H_
#include "pll.h"
#endif

/**
 * PLL Tree utils module
 * Prefix: pll_tree_, pll_utree_, pll_rtree_
 */
#define PLLMOD_TREE_DEFAULT_BRANCH_LENGTH 0.1

/* error codes (for this module, 3000-4000) ; B = 2^10+2^11*/
/* TBR errors (B + {2^2,2^1,2^0}) */
#define PLLMOD_TREE_ERROR_TBR_LEAF_BISECTION   3073 // B + {001}
#define PLLMOD_TREE_ERROR_TBR_OVERLAPPED_NODES 3074 // B + {010}
#define PLLMOD_TREE_ERROR_TBR_SAME_SUBTREE     3075 // B + {011}
#define PLLMOD_TREE_ERROR_TBR_MASK             3079 // B + {111}

/* NNI errors (B + {2^4,2^3}) */
#define PLLMOD_TREE_ERROR_NNI_INVALID_MOVE     3080 // B + {01...}
#define PLLMOD_TREE_ERROR_NNI_MASK             3096 // B + {11...}

/* SPR errors (B + {2^6,2^5}) */
#define PLLMOD_TREE_ERROR_SPR_INVALID_NODE     3104 // B + {01...}
#define PLLMOD_TREE_ERROR_SPR_MASK             3168 // B + {11...}

/* general errors (B + {2^8,2^7}) */
#define PLLMOD_TREE_ERROR_INTERCHANGE_LEAF     3200 // B + {01...}
#define PLLMOD_TREE_ERROR_INVALID_REARRAGE     3328 // B + {10...}
#define PLLMOD_TREE_ERROR_INVALID_TREE_SIZE    3456 // B + {10...}
#define PLLMOD_TREE_ERROR_INVALID_TREE         3584 // B + {10...}

#define PLLMOD_TREE_REARRANGE_SPR  0
#define PLLMOD_TREE_REARRANGE_NNI  1
#define PLLMOD_TREE_REARRANGE_TBR  2

#define PLLMOD_TREEINFO_PARTITION_ALL -1

typedef unsigned int pll_split_base_t;
typedef pll_split_base_t * pll_split_t;

typedef struct pll_tree_edge
{
  union
  {
    struct
    {
      pll_utree_t * parent;
      pll_utree_t * child;
    } utree;
    struct
    {
      pll_rtree_t * parent;
      pll_rtree_t * child;
    } rtree;
  } edge;
    double length;
} pll_tree_edge_t;

typedef struct
{
  int rearrange_type;
  int rooted;
  double  likelihood;

  union {
    struct {
      void * prune_edge;
      void * regraft_edge;
      double prune_bl;        //! length of the pruned branch
      double prune_left_bl;   //! length of the removed branch when pruning
      double prune_right_bl;  //! length of the removed branch when pruning
      double regraft_bl;      //! length of the splitted branch when regrafting
    } SPR;
    struct {
      void * edge;
      double left_left_bl;
      double left_right_bl;
      double right_left_bl;
      double right_right_bl;
      double edge_bl;
      int type;
    } NNI;
    struct {
      void * bisect_edge;
      pll_tree_edge_t reconn_edge;
      double bisect_left_bl;
      double bisect_right_bl;
      double reconn_parent_left_bl;
      double reconn_parent_right_bl;
      double reconn_child_left_bl;
      double reconn_child_right_bl;
    } TBR;
  };
} pll_tree_rollback_t;


typedef struct treeinfo
{
  // dimensions
  unsigned int tip_count;
  unsigned int partition_count;

  /* 0 = unlinked/per-partion branch lengths, 1 = linked/shared */
  int linked_branches;

  pll_utree_t * root;

  // partitions & partition-specific stuff
  pll_partition_t ** partitions;
  double * alphas;
  unsigned int ** param_indices;
  int ** subst_matrix_symmetries;
  double ** branch_lengths;

  /* precomputation buffers for derivatives (aka "sumtable") */
  double ** deriv_precomp;

  // invalidation flags
  char ** clv_valid;
  char ** pmatrix_valid;

  // buffers
  pll_utree_t ** travbuffer;
  unsigned int * matrix_indices;
  pll_operation_t * operations;

  // partition on which all operations should be performed
  int active_partition;

  // general-purpose counter
  unsigned int counter;
} pllmod_treeinfo_t;

/* Topological rearrangements */
/* functions at pll_tree.c */

PLL_EXPORT int pllmod_utree_tbr(pll_utree_t * b_edge,
                                pll_tree_edge_t * r_edge,
                                pll_tree_rollback_t * rollback_info);

PLL_EXPORT int pllmod_utree_spr(pll_utree_t * p_edge,
                                pll_utree_t * r_edge,
                                pll_tree_rollback_t * rollback_info);

/* type = {PLL_NNI_NEXT, PLL_NNI_NEXTNEXT} */
PLL_EXPORT int pllmod_utree_nni(pll_utree_t * edge,
                                int type,
                                pll_tree_rollback_t * rollback_info);

PLL_EXPORT int pllmod_tree_rollback(pll_tree_rollback_t * rollback_info);



/* Topological operations */

/* functions at rtree_operations.c */

PLL_EXPORT int pllmod_rtree_spr(pll_rtree_t * p_node,
                                pll_rtree_t * r_tree,
                                pll_rtree_t ** root,
                                pll_tree_rollback_t * rollback_info);

PLL_EXPORT int pllmod_rtree_get_sibling_pointers(pll_rtree_t * node,
                                                 pll_rtree_t ***self,
                                                 pll_rtree_t ***sister);

PLL_EXPORT pll_rtree_t * pllmod_rtree_prune(pll_rtree_t * node);

PLL_EXPORT int pllmod_rtree_regraft(pll_rtree_t * node,
                                    pll_rtree_t * tree);

/* functions at utree_operations.c */

PLL_EXPORT int pllmod_utree_bisect(pll_utree_t * edge,
                                   pll_utree_t ** parent_subtree,
                                   pll_utree_t ** child_subtree);

PLL_EXPORT pll_tree_edge_t pllmod_utree_reconnect(pll_tree_edge_t * edge,
                                                  pll_utree_t * pruned_edge);

PLL_EXPORT pll_utree_t * pllmod_utree_prune(pll_utree_t * edge);

PLL_EXPORT int pllmod_utree_regraft(pll_utree_t * edge,
                                    pll_utree_t * tree);

PLL_EXPORT int pllmod_utree_interchange(pll_utree_t * edge1,
                                        pll_utree_t * edge2);

PLL_EXPORT pll_utree_t * pllmod_utree_create_node(unsigned int clv_index,
                                                  int scaler_index,
                                                  char * label,
                                                  void * data);

PLL_EXPORT int pllmod_utree_connect_nodes(pll_utree_t * parent,
                                          pll_utree_t * child,
                                           double length);

/* Topological search */

/* functions at rtree_operations.c */

PLL_EXPORT int pllmod_rtree_nodes_at_node_dist(pll_rtree_t * root,
                                               pll_rtree_t ** outbuffer,
                                               unsigned int * n_nodes,
                                               int min_distance,
                                               int max_distance);

/* functions at utree_operations.c */

PLL_EXPORT int pllmod_utree_nodes_at_node_dist(pll_utree_t * node,
                                            pll_utree_t ** outbuffer,
                                            unsigned int * n_nodes,
                                            unsigned int min_distance,
                                            unsigned int max_distance);

PLL_EXPORT int pllmod_utree_nodes_at_edge_dist(pll_utree_t * edge,
                                               pll_utree_t ** outbuffer,
                                               unsigned int * n_nodes,
                                               unsigned int min_distance,
                                               unsigned int max_distance);

/* Tree construction */
/* functions at pll_tree.c */

PLL_EXPORT pll_utree_t * pllmod_utree_create_random(unsigned int n_taxa,
                                                    const char * const* names);



/* Discrete operations */
/* functions at utree_distances.c */

PLL_EXPORT unsigned int pllmod_utree_rf_distance(pll_utree_t * t1,
                                                 pll_utree_t * t2,
                                                 unsigned int n_tips);

/* check that node ids and tip labels agree in both trees */
PLL_EXPORT int pllmod_utree_consistency_check(pll_utree_t * t1,
                                              pll_utree_t * t2,
                                              unsigned int n_tips);

/* if 2 different trees are parsed from newick node ids migh have been set
   in a different order, so this function sets node ids in t2 such that
   node ids and tip labels agree in both trees */
PLL_EXPORT int pllmod_utree_consistency_set(pll_utree_t * t1,
                                            pll_utree_t * t2,
                                            unsigned int n_tips);

PLL_EXPORT unsigned int pllmod_utree_split_rf_distance(pll_split_t * s1,
                                                       pll_split_t * s2,
                                                       unsigned int n_tips);

PLL_EXPORT pll_split_t * pllmod_utree_split_create(pll_utree_t * tree,
                                                   unsigned int n_tips,
                                                   unsigned int * n_splits);

PLL_EXPORT void pllmod_utree_split_show(pll_split_t split, unsigned int n_tips);

PLL_EXPORT void pllmod_utree_split_destroy(pll_split_t * split_list);



/* Additional utilities */
/* functions at pll_tree.c */

PLL_EXPORT int pllmod_utree_set_clv_minimal(pll_utree_t * root,
                                         unsigned int n_tips);

PLL_EXPORT int pllmod_utree_traverse_apply(pll_utree_t * root,
                                        int (*cb_pre_trav)(pll_utree_t *,
                                                           void *),
                                        int (*cb_post_trav)(pll_utree_t *,
                                                            void *),
                                        void *data);

PLL_EXPORT int pllmod_utree_is_tip(pll_utree_t * node);

PLL_EXPORT void pllmod_utree_set_length(pll_utree_t * edge,
                                     double length);

PLL_EXPORT void pllmod_utree_scale_branches(pll_utree_t * tree,
                                         double branch_length_scaler);

PLL_EXPORT double pllmod_utree_compute_lk(pll_partition_t * partition,
                                       pll_utree_t * tree,
                                       const unsigned int * params_indices,
                                       int update_pmatrices,
                                       int update_partials);

PLL_EXPORT int pllmod_rtree_traverse_apply(pll_rtree_t * root,
                                           int (*cb_pre_trav)(pll_rtree_t *,
                                                              void *),
                                           int (*cb_in_trav)(pll_rtree_t *,
                                                             void *),
                                           int (*cb_post_trav)(pll_rtree_t *,
                                                               void *),
                                           void *data);

/* treeinfo */

PLL_EXPORT pllmod_treeinfo_t * pllmod_treeinfo_create(pll_utree_t * root,
                                                     unsigned int tips,
                                                     unsigned int partitions,
                                                     int linked_branch_lengths);

PLL_EXPORT int pllmod_treeinfo_init_partition(pllmod_treeinfo_t * treeinfo,
                                           unsigned int partition_index,
                                           pll_partition_t * partition,
                                           double alpha,
                                           const unsigned int * param_indices,
                                           const int * subst_matrix_symmetries);

PLL_EXPORT int pllmod_treeinfo_set_active_partition(pllmod_treeinfo_t * treeinfo,
                                                    int partition_index);

PLL_EXPORT void pllmod_treeinfo_set_root(pllmod_treeinfo_t * treeinfo,
                                         pll_utree_t * root);

PLL_EXPORT void pllmod_treeinfo_set_branch_length(pllmod_treeinfo_t * treeinfo,
                                                  pll_utree_t * edge,
                                                  double length);

PLL_EXPORT int pllmod_treeinfo_destroy_partition(pllmod_treeinfo_t * treeinfo,
                                                 unsigned int partition_index);

PLL_EXPORT void pllmod_treeinfo_destroy(pllmod_treeinfo_t * treeinfo);

PLL_EXPORT int pllmod_treeinfo_update_prob_matrices(pllmod_treeinfo_t * treeinfo,
                                                    int update_all);

PLL_EXPORT void pllmod_treeinfo_invalidate_all(pllmod_treeinfo_t * treeinfo);

PLL_EXPORT int pllmod_treeinfo_validate_clvs(pllmod_treeinfo_t * treeinfo,
                                             pll_utree_t ** travbuffer,
                                             unsigned int travbuffer_size);

PLL_EXPORT void pllmod_treeinfo_invalidate_pmatrix(pllmod_treeinfo_t * treeinfo,
                                                   const pll_utree_t * edge);

PLL_EXPORT void pllmod_treeinfo_invalidate_clv(pllmod_treeinfo_t * treeinfo,
                                               const pll_utree_t * edge);

PLL_EXPORT double pllmod_treeinfo_compute_loglh(pllmod_treeinfo_t * treeinfo,
                                                int incremental);

#endif /* PLL_TREE_H_ */

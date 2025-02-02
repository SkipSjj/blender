/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup spoutliner
 */

#include <cstring>

#include "BLI_listbase.h"
#include "BLI_listbase_wrapper.hh"
#include "BLI_utildefines.h"

#include "DNA_sequence_types.h"
#include "DNA_space_types.h"

#include "SEQ_sequencer.hh"

#include "../outliner_intern.hh"
#include "tree_display.hh"

namespace blender::ed::outliner {

template<typename T> using List = ListBaseWrapper<T>;

TreeDisplaySequencer::TreeDisplaySequencer(SpaceOutliner &space_outliner)
    : AbstractTreeDisplay(space_outliner)
{
}

ListBase TreeDisplaySequencer::build_tree(const TreeSourceData &source_data)
{
  ListBase tree = {nullptr};

  Editing *ed = SEQ_editing_get(source_data.scene);
  if (ed == nullptr) {
    return tree;
  }

  for (Sequence *seq : List<Sequence>(ed->seqbasep)) {
    SequenceAddOp op = need_add_seq_dup(seq);
    if (op == SEQUENCE_DUPLICATE_NONE) {
      add_element(&tree, nullptr, seq, nullptr, TSE_SEQUENCE, 0);
    }
    else if (op == SEQUENCE_DUPLICATE_ADD) {
      TreeElement *te = add_element(&tree, nullptr, seq, nullptr, TSE_SEQUENCE_DUP, 0);
      add_seq_dup(seq, te, 0);
    }
  }

  return tree;
}

SequenceAddOp TreeDisplaySequencer::need_add_seq_dup(Sequence *seq) const
{
  if ((!seq->data) || (!seq->data->stripdata)) {
    return SEQUENCE_DUPLICATE_NONE;
  }

  /*
   * First check backward, if we found a duplicate
   * sequence before this, don't need it, just return.
   */
  Sequence *p = seq->prev;
  while (p) {
    if ((!p->data) || (!p->data->stripdata)) {
      p = p->prev;
      continue;
    }

    if (STREQ(p->data->stripdata->filename, seq->data->stripdata->filename)) {
      return SEQUENCE_DUPLICATE_NOOP;
    }
    p = p->prev;
  }

  p = seq->next;
  while (p) {
    if ((!p->data) || (!p->data->stripdata)) {
      p = p->next;
      continue;
    }

    if (STREQ(p->data->stripdata->filename, seq->data->stripdata->filename)) {
      return SEQUENCE_DUPLICATE_ADD;
    }
    p = p->next;
  }

  return SEQUENCE_DUPLICATE_NONE;
}

void TreeDisplaySequencer::add_seq_dup(Sequence *seq, TreeElement *te, short index)
{
  Sequence *p = seq;
  while (p) {
    if ((!p->data) || (!p->data->stripdata) || (p->data->stripdata->filename[0] == '\0')) {
      p = p->next;
      continue;
    }

    if (STREQ(p->data->stripdata->filename, seq->data->stripdata->filename)) {
      add_element(&te->subtree, nullptr, (void *)p, te, TSE_SEQUENCE, index);
    }
    p = p->next;
  }
}

}  // namespace blender::ed::outliner

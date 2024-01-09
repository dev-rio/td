//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2024
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/telegram/SavedMessagesTopicId.h"

#include "td/telegram/AccessRights.h"
#include "td/telegram/Dependencies.h"
#include "td/telegram/DialogManager.h"
#include "td/telegram/MessageForwardInfo.h"
#include "td/telegram/MessagesManager.h"
#include "td/telegram/Td.h"

namespace td {

static constexpr DialogId HIDDEN_AUTHOR_DIALOG_ID = DialogId(static_cast<int64>(2666000));

SavedMessagesTopicId::SavedMessagesTopicId(DialogId my_dialog_id, const MessageForwardInfo *message_forward_info) {
  if (message_forward_info != nullptr) {
    auto last_dialog_id = message_forward_info->get_last_dialog_id();
    if (last_dialog_id.is_valid()) {
      dialog_id_ = last_dialog_id;
      return;
    }
    auto from_dialog_id = message_forward_info->get_origin().get_sender();
    if (from_dialog_id.is_valid()) {
      dialog_id_ = my_dialog_id;
      return;
    }
    if (message_forward_info->get_origin().is_sender_hidden()) {
      dialog_id_ = HIDDEN_AUTHOR_DIALOG_ID;
    }
  }
  dialog_id_ = my_dialog_id;
}

SavedMessagesTopicId::SavedMessagesTopicId(const Td *td,
                                           const td_api::object_ptr<td_api::SavedMessagesTopic> &saved_messages_topic) {
  if (saved_messages_topic == nullptr) {
    return;
  }
  switch (saved_messages_topic->get_id()) {
    case td_api::savedMessagesTopicMyNotes::ID:
      dialog_id_ = td->dialog_manager_->get_my_dialog_id();
      break;
    case td_api::savedMessagesTopicAuthorHidden::ID:
      dialog_id_ = HIDDEN_AUTHOR_DIALOG_ID;
      break;
    case td_api::savedMessagesTopicSavedFromChat::ID:
      dialog_id_ =
          DialogId(static_cast<const td_api::savedMessagesTopicSavedFromChat *>(saved_messages_topic.get())->chat_id_);
      break;
    default:
      UNREACHABLE();
      break;
  }
}

td_api::object_ptr<td_api::SavedMessagesTopic> SavedMessagesTopicId::get_saved_messages_topic_object(Td *td) const {
  if (dialog_id_ == DialogId()) {
    return nullptr;
  }
  if (dialog_id_ == td->dialog_manager_->get_my_dialog_id()) {
    return td_api::make_object<td_api::savedMessagesTopicMyNotes>();
  }
  if (dialog_id_ == HIDDEN_AUTHOR_DIALOG_ID) {
    return td_api::make_object<td_api::savedMessagesTopicAuthorHidden>();
  }
  return td_api::make_object<td_api::savedMessagesTopicSavedFromChat>(
      td->messages_manager_->get_chat_id_object(dialog_id_, "savedMessagesTopicSavedFromChat"));
}

bool SavedMessagesTopicId::have_input_peer(Td *td) const {
  if (dialog_id_.get_type() == DialogType::SecretChat ||
      !td->dialog_manager_->have_dialog_info_force(dialog_id_, "SavedMessagesTopicId::have_input_peer")) {
    return false;
  }
  return td->dialog_manager_->have_input_peer(dialog_id_, AccessRights::Know);
}

Status SavedMessagesTopicId::is_valid_status(Td *td) const {
  if (!dialog_id_.is_valid()) {
    return Status::Error(400, "Invalid Saved Messages topic specified");
  }
  if (!have_input_peer(td)) {
    return Status::Error(400, "Invalid Saved Messages topic specified");
  }
  return Status::OK();
}

Status SavedMessagesTopicId::is_valid_in(Td *td, DialogId dialog_id) const {
  if (dialog_id_ != DialogId()) {
    if (dialog_id != td->dialog_manager_->get_my_dialog_id()) {
      return Status::Error(400, "Can't use Saved Messages topic in the chat");
    }
    if (!have_input_peer(td)) {
      return Status::Error(400, "Invalid Saved Messages topic specified");
    }
  }
  return Status::OK();
}

telegram_api::object_ptr<telegram_api::InputPeer> SavedMessagesTopicId::get_input_peer(const Td *td) const {
  return td->dialog_manager_->get_input_peer(dialog_id_, AccessRights::Know);
}

telegram_api::object_ptr<telegram_api::InputDialogPeer> SavedMessagesTopicId::get_input_dialog_peer(
    const Td *td) const {
  return telegram_api::make_object<telegram_api::inputDialogPeer>(get_input_peer(td));
}

void SavedMessagesTopicId::add_dependencies(Dependencies &dependencies) const {
  if (dialog_id_ == HIDDEN_AUTHOR_DIALOG_ID) {
    dependencies.add_dialog_dependencies(dialog_id_);
  } else {
    dependencies.add_dialog_and_dependencies(dialog_id_);
  }
}

StringBuilder &operator<<(StringBuilder &string_builder, SavedMessagesTopicId saved_messages_topic_id) {
  if (!saved_messages_topic_id.dialog_id_.is_valid()) {
    return string_builder << "[no Saved Messages topic]";
  }
  if (saved_messages_topic_id.dialog_id_ == HIDDEN_AUTHOR_DIALOG_ID) {
    return string_builder << "[Author Hidden topic]";
  }
  return string_builder << "[topic of " << saved_messages_topic_id.dialog_id_ << ']';
}

}  // namespace td
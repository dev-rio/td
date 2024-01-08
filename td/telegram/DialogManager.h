//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/AccentColorId.h"
#include "td/telegram/AccessRights.h"
#include "td/telegram/ChannelId.h"
#include "td/telegram/CustomEmojiId.h"
#include "td/telegram/DialogId.h"
#include "td/telegram/DialogParticipant.h"
#include "td/telegram/EmojiStatus.h"
#include "td/telegram/files/FileId.h"
#include "td/telegram/InputDialogId.h"
#include "td/telegram/MessageId.h"
#include "td/telegram/NotificationSettingsScope.h"
#include "td/telegram/Photo.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/actor/actor.h"

#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/FlatHashSet.h"
#include "td/utils/Promise.h"
#include "td/utils/Status.h"
#include "td/utils/WaitFreeHashMap.h"

#include <memory>
#include <utility>

namespace td {

class ReportReason;
class Td;
class Usernames;

class DialogManager final : public Actor {
 public:
  DialogManager(Td *td, ActorShared<> parent);
  DialogManager(const DialogManager &) = delete;
  DialogManager &operator=(const DialogManager &) = delete;
  DialogManager(DialogManager &&) = delete;
  DialogManager &operator=(DialogManager &&) = delete;
  ~DialogManager() final;

  DialogId get_my_dialog_id() const;

  InputDialogId get_input_dialog_id(DialogId dialog_id) const;

  tl_object_ptr<telegram_api::InputPeer> get_input_peer(DialogId dialog_id, AccessRights access_rights) const;

  static tl_object_ptr<telegram_api::InputPeer> get_input_peer_force(DialogId dialog_id);

  vector<tl_object_ptr<telegram_api::InputPeer>> get_input_peers(const vector<DialogId> &dialog_ids,
                                                                 AccessRights access_rights) const;

  tl_object_ptr<telegram_api::InputDialogPeer> get_input_dialog_peer(DialogId dialog_id,
                                                                     AccessRights access_rights) const;

  vector<tl_object_ptr<telegram_api::InputDialogPeer>> get_input_dialog_peers(const vector<DialogId> &dialog_ids,
                                                                              AccessRights access_rights) const;

  tl_object_ptr<telegram_api::inputEncryptedChat> get_input_encrypted_chat(DialogId dialog_id,
                                                                           AccessRights access_rights) const;

  bool have_input_peer(DialogId dialog_id, AccessRights access_rights) const;

  bool have_dialog_force(DialogId dialog_id, const char *source) const;

  void force_create_dialog(DialogId dialog_id, const char *source, bool expect_no_access = false,
                           bool force_update_dialog_pos = false);

  vector<DialogId> get_peers_dialog_ids(vector<telegram_api::object_ptr<telegram_api::Peer>> &&peers,
                                        bool expect_no_access = false);

  bool have_dialog_info(DialogId dialog_id) const;

  bool have_dialog_info_force(DialogId dialog_id, const char *source) const;

  void get_dialog_info_full(DialogId dialog_id, Promise<Unit> &&promise, const char *source);

  void reload_dialog_info_full(DialogId dialog_id, const char *source);

  void on_dialog_info_full_invalidated(DialogId dialog_id);

  int64 get_chat_id_object(DialogId dialog_id, const char *source) const;

  vector<int64> get_chat_ids_object(const vector<DialogId> &dialog_ids, const char *source) const;

  td_api::object_ptr<td_api::chats> get_chats_object(int32 total_count, const vector<DialogId> &dialog_ids,
                                                     const char *source) const;

  td_api::object_ptr<td_api::chats> get_chats_object(const std::pair<int32, vector<DialogId>> &dialog_ids,
                                                     const char *source) const;

  td_api::object_ptr<td_api::ChatType> get_chat_type_object(DialogId dialog_id) const;

  NotificationSettingsScope get_dialog_notification_setting_scope(DialogId dialog_id) const;

  bool is_anonymous_administrator(DialogId dialog_id, string *author_signature) const;

  bool is_group_dialog(DialogId dialog_id) const;

  bool is_forum_channel(DialogId dialog_id) const;

  bool is_broadcast_channel(DialogId dialog_id) const;

  bool on_get_dialog_error(DialogId dialog_id, const Status &status, const char *source);

  void delete_dialog(DialogId dialog_id, Promise<Unit> &&promise);

  string get_dialog_title(DialogId dialog_id) const;

  const DialogPhoto *get_dialog_photo(DialogId dialog_id) const;

  int32 get_dialog_accent_color_id_object(DialogId dialog_id) const;

  CustomEmojiId get_dialog_background_custom_emoji_id(DialogId dialog_id) const;

  int32 get_dialog_profile_accent_color_id_object(DialogId dialog_id) const;

  CustomEmojiId get_dialog_profile_background_custom_emoji_id(DialogId dialog_id) const;

  RestrictedRights get_dialog_default_permissions(DialogId dialog_id) const;

  td_api::object_ptr<td_api::emojiStatus> get_dialog_emoji_status_object(DialogId dialog_id) const;

  bool get_dialog_has_protected_content(DialogId dialog_id) const;

  bool is_dialog_action_unneeded(DialogId dialog_id) const;

  void set_dialog_title(DialogId dialog_id, const string &title, Promise<Unit> &&promise);

  void set_dialog_photo(DialogId dialog_id, const td_api::object_ptr<td_api::InputChatPhoto> &input_photo,
                        Promise<Unit> &&promise);

  void set_dialog_accent_color(DialogId dialog_id, AccentColorId accent_color_id,
                               CustomEmojiId background_custom_emoji_id, Promise<Unit> &&promise);

  void set_dialog_profile_accent_color(DialogId dialog_id, AccentColorId profile_accent_color_id,
                                       CustomEmojiId profile_background_custom_emoji_id, Promise<Unit> &&promise);

  void set_dialog_permissions(DialogId dialog_id, const td_api::object_ptr<td_api::chatPermissions> &permissions,
                              Promise<Unit> &&promise);

  void set_dialog_emoji_status(DialogId dialog_id, const EmojiStatus &emoji_status, Promise<Unit> &&promise);

  void toggle_dialog_has_protected_content(DialogId dialog_id, bool has_protected_content, Promise<Unit> &&promise);

  void set_dialog_description(DialogId dialog_id, const string &description, Promise<Unit> &&promise);

  bool can_report_dialog(DialogId dialog_id) const;

  void report_dialog(DialogId dialog_id, const vector<MessageId> &message_ids, ReportReason &&reason,
                     Promise<Unit> &&promise);

  void report_dialog_photo(DialogId dialog_id, FileId file_id, ReportReason &&reason, Promise<Unit> &&promise);

  Status can_pin_messages(DialogId dialog_id) const;

  bool is_dialog_removed_from_dialog_list(DialogId dialog_id) const;

  void upload_dialog_photo(DialogId dialog_id, FileId file_id, bool is_animation, double main_frame_timestamp,
                           bool is_reupload, Promise<Unit> &&promise, vector<int> bad_parts = {});

  void on_dialog_usernames_updated(DialogId dialog_id, const Usernames &old_usernames, const Usernames &new_usernames);

  void on_dialog_usernames_received(DialogId dialog_id, const Usernames &usernames, bool from_database);

  void resolve_dialog(const string &username, ChannelId channel_id, Promise<DialogId> promise);

  DialogId get_resolved_dialog_by_username(const string &username) const;

  DialogId resolve_dialog_username(const string &username, Promise<Unit> &promise);

  DialogId search_public_dialog(const string &username_to_search, bool force, Promise<Unit> &&promise);

  void reload_voice_chat_on_search(const string &username);

  void on_resolved_username(const string &username, DialogId dialog_id);

  void drop_username(const string &username);

 private:
  static constexpr size_t MAX_TITLE_LENGTH = 128;  // server side limit for chat title

  static constexpr int32 USERNAME_CACHE_EXPIRE_TIME = 86400;

  void tear_down() final;

  void on_upload_dialog_photo(FileId file_id, telegram_api::object_ptr<telegram_api::InputFile> input_file);

  void on_upload_dialog_photo_error(FileId file_id, Status status);

  void send_edit_dialog_photo_query(DialogId dialog_id, FileId file_id,
                                    telegram_api::object_ptr<telegram_api::InputChatPhoto> &&input_chat_photo,
                                    Promise<Unit> &&promise);

  void send_resolve_dialog_username_query(const string &username, Promise<Unit> &&promise);

  void on_resolve_dialog(const string &username, ChannelId channel_id, Promise<DialogId> &&promise);

  class UploadDialogPhotoCallback;
  std::shared_ptr<UploadDialogPhotoCallback> upload_dialog_photo_callback_;

  struct UploadedDialogPhotoInfo {
    DialogId dialog_id;
    double main_frame_timestamp;
    bool is_animation;
    bool is_reupload;
    Promise<Unit> promise;

    UploadedDialogPhotoInfo(DialogId dialog_id, double main_frame_timestamp, bool is_animation, bool is_reupload,
                            Promise<Unit> promise)
        : dialog_id(dialog_id)
        , main_frame_timestamp(main_frame_timestamp)
        , is_animation(is_animation)
        , is_reupload(is_reupload)
        , promise(std::move(promise)) {
    }
  };
  FlatHashMap<FileId, UploadedDialogPhotoInfo, FileIdHash> being_uploaded_dialog_photos_;

  struct ResolvedUsername {
    DialogId dialog_id;
    double expires_at = 0.0;

    ResolvedUsername() = default;
    ResolvedUsername(DialogId dialog_id, double expires_at) : dialog_id(dialog_id), expires_at(expires_at) {
    }
  };
  WaitFreeHashMap<string, ResolvedUsername> resolved_usernames_;
  WaitFreeHashMap<string, DialogId> inaccessible_resolved_usernames_;
  FlatHashSet<string> reload_voice_chat_on_search_usernames_;

  Td *td_;
  ActorShared<> parent_;
};

}  // namespace td

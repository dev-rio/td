//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2022
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/CustomEmojiId.h"
#include "td/telegram/DialogId.h"
#include "td/telegram/ForumTopicEditedData.h"
#include "td/telegram/ForumTopicInfo.h"
#include "td/telegram/MessageId.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/actor/actor.h"

#include "td/utils/common.h"
#include "td/utils/Promise.h"
#include "td/utils/Status.h"
#include "td/utils/WaitFreeHashMap.h"

namespace td {

class Td;

class ForumTopicManager final : public Actor {
 public:
  ForumTopicManager(Td *td, ActorShared<> parent);
  ForumTopicManager(const ForumTopicManager &) = delete;
  ForumTopicManager &operator=(const ForumTopicManager &) = delete;
  ForumTopicManager(ForumTopicManager &&) = delete;
  ForumTopicManager &operator=(ForumTopicManager &&) = delete;
  ~ForumTopicManager() final;

  void create_forum_topic(DialogId dialog_id, string &&title, td_api::object_ptr<td_api::forumTopicIcon> &&icon,
                          Promise<td_api::object_ptr<td_api::forumTopicInfo>> &&promise);

  void on_forum_topic_created(DialogId dialog_id, unique_ptr<ForumTopicInfo> &&forum_topic_info,
                              Promise<td_api::object_ptr<td_api::forumTopicInfo>> &&promise);

  void edit_forum_topic(DialogId dialog_id, MessageId top_thread_message_id, string &&title,
                        bool edit_icon_custom_emoji, CustomEmojiId icon_custom_emoji_id, Promise<Unit> &&promise);

  void get_forum_topic(DialogId dialog_id, MessageId top_thread_message_id,
                       Promise<td_api::object_ptr<td_api::forumTopic>> &&promise);

  void toggle_forum_topic_is_closed(DialogId dialog_id, MessageId top_thread_message_id, bool is_closed,
                                    Promise<Unit> &&promise);

  void toggle_forum_topic_is_hidden(DialogId dialog_id, bool is_hidden, Promise<Unit> &&promise);

  void delete_forum_topic(DialogId dialog_id, MessageId top_thread_message_id, Promise<Unit> &&promise);

  void delete_all_dialog_topics(DialogId dialog_id);

  void on_forum_topic_edited(DialogId dialog_id, MessageId top_thread_message_id,
                             const ForumTopicEditedData &edited_data);

  void on_get_forum_topics(DialogId dialog_id, vector<tl_object_ptr<telegram_api::ForumTopic>> &&forum_topics,
                           const char *source);

  void on_topic_message_count_changed(DialogId dialog_id, MessageId top_thread_message_id, int diff);

 private:
  static constexpr size_t MAX_FORUM_TOPIC_TITLE_LENGTH = 128;  // server side limit for forum topic title

  struct Topic {
    unique_ptr<ForumTopicInfo> info_;
    int32 message_count_ = 0;

    template <class StorerT>
    void store(StorerT &storer) const;

    template <class ParserT>
    void parse(ParserT &parser);

    int32 MAGIC = 0x1fac3901;
  };

  struct DialogTopics {
    WaitFreeHashMap<MessageId, unique_ptr<Topic>, MessageIdHash> topics_;
  };

  void tear_down() final;

  Status is_forum(DialogId dialog_id);

  bool can_be_forum(DialogId dialog_id) const;

  DialogTopics *add_dialog_topics(DialogId dialog_id);

  static Topic *add_topic(DialogTopics *dialog_topics, MessageId top_thread_message_id);

  Topic *add_topic(DialogId dialog_id, MessageId top_thread_message_id);

  Topic *get_topic(DialogId dialog_id, MessageId top_thread_message_id);

  const Topic *get_topic(DialogId dialog_id, MessageId top_thread_message_id) const;

  ForumTopicInfo *get_topic_info(DialogId dialog_id, MessageId top_thread_message_id);

  const ForumTopicInfo *get_topic_info(DialogId dialog_id, MessageId top_thread_message_id) const;

  void on_delete_forum_topic(DialogId dialog_id, MessageId top_thread_message_id, Promise<Unit> &&promise);

  td_api::object_ptr<td_api::updateForumTopicInfo> get_update_forum_topic_info(DialogId dialog_id,
                                                                               const ForumTopicInfo *topic_info) const;

  void send_update_forum_topic_info(DialogId dialog_id, const ForumTopicInfo *topic_info) const;

  void save_topic_to_database(DialogId dialog_id, const Topic *topic);

  void delete_topic_from_database(DialogId dialog_id, MessageId top_thread_message_id, Promise<Unit> &&promise);

  Td *td_;
  ActorShared<> parent_;

  WaitFreeHashMap<DialogId, unique_ptr<DialogTopics>, DialogIdHash> dialog_topics_;
};

}  // namespace td

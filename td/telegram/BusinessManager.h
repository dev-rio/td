//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/DialogId.h"
#include "td/telegram/td_api.h"
#include "td/telegram/UserId.h"

#include "td/actor/actor.h"

#include "td/utils/common.h"
#include "td/utils/Promise.h"

namespace td {

class BusinessAwayMessage;
class BusinessGreetingMessage;
class BusinessIntro;
class BusinessWorkHours;
class DialogLocation;
class Td;

class BusinessManager final : public Actor {
 public:
  BusinessManager(Td *td, ActorShared<> parent);

  void get_business_connected_bot(Promise<td_api::object_ptr<td_api::businessConnectedBot>> &&promise);

  void set_business_connected_bot(td_api::object_ptr<td_api::businessConnectedBot> &&bot, Promise<Unit> &&promise);

  void delete_business_connected_bot(UserId bot_user_id, Promise<Unit> &&promise);

  void toggle_business_connected_bot_chat_is_paused(DialogId dialog_id, bool is_paused, Promise<Unit> &&promise);

  void set_business_location(DialogLocation &&location, Promise<Unit> &&promise);

  void set_business_work_hours(BusinessWorkHours &&work_hours, Promise<Unit> &&promise);

  void set_business_greeting_message(BusinessGreetingMessage &&greeting_message, Promise<Unit> &&promise);

  void set_business_away_message(BusinessAwayMessage &&away_message, Promise<Unit> &&promise);

  void set_business_intro(BusinessIntro &&intro, Promise<Unit> &&promise);

 private:
  void tear_down() final;

  Td *td_;
  ActorShared<> parent_;
};

}  // namespace td

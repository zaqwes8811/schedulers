#include "core/config.h"

//#include "common/app_types.h"
//#include "view/qt_event_loop.h"

#include "actor_ui.h"

namespace actors {
//void UIActor::Run() {
//  while( !m_done ) {
//    try {
//      {
//        if (uiPtr) {
//          auto pr = uiPtr->off();
//          if (!uiPtr->poll()) {
//             auto p = uiPtr.release();
//             delete p;
//          }

//          if (!uiPtr)
//            pr->set_value(0);
//        }
//      }

//      // ! can't sleep or wait!
//      // FIXME: It's danger work without UI
//      Message msg;
//      if (mq.try_pop(msg)) {
//        msg();            // execute message
//      }
//    } catch (fatal_error&) {
//      throw;
//    } catch (...) {
//      // FIXME: don't know what to do
//      throw;  // bad
//    }
//  } // note: last message sets done to true
//}

//std::future<int> UIActor::RunUI(concepts::db_manager_concept_t db) {
//  auto pr = std::make_shared<std::promise<int>>();
//  auto f = pr->get_future();

//  post([db, this, pr]() {
//    uiPtr = std::unique_ptr<actors::UiObject>(new actors::UiObject(db, pr));
//  });
//  return f;
//}

//UIActor::UIActor() : m_done(false), mq(100), uiPtr(NULL)
//{
//  //thd = std::unique_ptr<std::thread>(new std::thread( [=]{ this->Run(); } ) );
//}

//UIActor::~UIActor() {
//  //post( [&]{ m_done = true; } );
//  thd->join();
//}

}  // space



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *
 *  The basis of this file has been automatically generated
 *  by the TeMoto action package generator. Modify this file
 *  as you wish but please note:
 *
 *    WE HIGHLIY RECOMMEND TO REFER TO THE TeMoto ACTION
 *    IMPLEMENTATION TUTORIAL IF YOU ARE UNFAMILIAR WITH
 *    THE PROCESS OF CREATING CUSTOM TeMoto ACTION PACKAGES
 *    
 *  because there are plenty of components that should not be
 *  modified or which do not make sence at the first glance.
 *
 *  See TeMoto documentation & tutorials at: 
 *    https://temoto-telerobotics.github.io
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <class_loader/class_loader.hpp>
#include "ta_ros1_actor_synchronizer/temoto_action.h"
#include "ros/ros.h"
#include "temoto_action_engine/GetUmrfGraphs.h"
#include "temoto_action_engine/umrf_json_converter.h"

/* 
 * ACTION IMPLEMENTATION of TaRos1ActorSynchronizer 
 */
class TaRos1ActorSynchronizer : public TemotoAction
{
public:

/*
 * Function that gets invoked only once (when the action is initialized) throughout the action's lifecycle
 */
void initializeTemotoAction()
{
  remote_action_name_ = getUmrfNodeConst().getFullName(); 
  remote_actor_srv_name_ = "/" + getUmrfNodeConst().getActor() + "/get_umrf_graphs";
  get_umrf_graphs_client_ = nh_.serviceClient<temoto_action_engine::GetUmrfGraphs>(remote_actor_srv_name_);
  TEMOTO_INFO_STREAM("Action initialized");
}

/*
 * Function that gets invoked when the action is executed (REQUIRED)
 */
void executeTemotoAction()
{
  TEMOTO_INFO_STREAM("Fetching updates about action '" << remote_action_name_ << "' from '" << remote_actor_srv_name_ << "' ...");

  while (actionOk())
  {
    temoto_action_engine::GetUmrfGraphs graphs_query;
    if (!get_umrf_graphs_client_.call(graphs_query))
    {
      TEMOTO_WARN_STREAM("Could not reach server: '" << remote_actor_srv_name_ << "'");
      ros::Duration(0.5).sleep();
      continue;  
    }

    // Go thrhough the graphs and ...
    bool remote_action_found = false;
    for (const auto& umrf_graph_json : graphs_query.response.umrf_graph_jsons)
    {
      // ... look for this specific action
      UmrfGraph remote_umrf_graph = umrf_json_converter::fromUmrfGraphJsonStr(umrf_graph_json);
      for (const auto& remote_umrf_node : remote_umrf_graph.getUmrfNodes())
      {
        if (remote_umrf_node.getFullName() != remote_action_name_)
        {
          continue;
        }
        
        remote_action_found = true;
        UmrfNode::State current_state = remote_umrf_node.getState();

        // Check if the action is finished
        if (!((current_state == UmrfNode::State::FINISHED || current_state == UmrfNode::State::UNINITIALIZED) 
          && previous_state_ == UmrfNode::State::RUNNING))
        {
          //TEMOTO_INFO_STREAM("'" << remote_action_name_ << "' is not finished yet (state = " << remote_umrf_node.state_to_str_map_.at(remote_umrf_node.getState()) << ")");
          previous_state_ = current_state;
          break;
        }

        // We found the action we're looking for and it has finished
        TEMOTO_INFO_STREAM("'" << remote_action_name_ << "' has finished");
        getUmrfNode().setOutputParameters(remote_umrf_node.getOutputParameters());
        return;
      }
    }

    // So the logic is that if the action is not found, it must be finished because the graph is erased
    if (!remote_action_found)
    {
      TEMOTO_INFO_STREAM("'" << remote_action_name_ << "' has finished");
      return;
    }

    ros::Duration(0.5).sleep();
  }
}

// Destructor
~TaRos1ActorSynchronizer()
{
  TEMOTO_INFO("Action instance destructed");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Class members
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ros::NodeHandle nh_;
ros::ServiceClient get_umrf_graphs_client_;
std::string remote_actor_srv_name_;
std::string remote_action_name_;
UmrfNode::State previous_state_;

}; // TaRos1ActorSynchronizer class

/* REQUIRED BY CLASS LOADER */
CLASS_LOADER_REGISTER_CLASS(TaRos1ActorSynchronizer, ActionBase);

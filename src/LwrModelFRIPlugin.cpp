#include "LwrModelFRIPlugin.h"

// system includes
#include <stdio.h>

// library includes
#include <boost/bind.hpp>

// custom includes
#include <lwr/LwrLibrary.hpp>
#include <ahbstring.h>


namespace gazebo
{
/*---------------------------------- public: -----------------------------{{{-*/
  void
  LwrModelFRIPlugin::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf) 
  {
    std::cout << "------------------- LwrModelFRIPlugin -------------------" << std::endl;

    m_model = _parent;

    m_updatePeriod = 0.001;

    if (!ros::isInitialized()) {
      ROS_FATAL_STREAM("LwrModelFRIPlugin: A ROS node for Gazebo has not been initialized, unable to load plugin. "
        << "Load the Gazebo system plugin 'libgazebo_ros_api_plugin.so' in the gazebo_ros package)");
      return;
    }

    if (!loadParams(_sdf)) {
      std::cout << "Error during loadParams" << std::endl;
      return;
    }

    m_node = new ros::NodeHandle(m_nodeName);
    m_lastUpdateTime = ros::Time::now();

    unsigned lwrNameLen = std::string("lwrN").size();
    std::string pluginName = _sdf->GetAttribute("name")->GetAsString();
    std::string lwrName = pluginName.substr(pluginName.size() - lwrNameLen);

#if 0
    std::cout << "_sdf: name=" << _sdf->GetName() << " attributeCount=" << _sdf->GetAttributeCount() << std::endl;
    std::cout << "Attributes: " << std::endl;
    for (unsigned attrIdx = 0; attrIdx < _sdf->GetAttributeCount(); attrIdx++) {
      sdf::ParamPtr param = _sdf->GetAttribute(attrIdx);
      std::cout << attrIdx << " key=" << param->GetKey() << " value=" << param->GetAsString() << std::endl;
    }

    std::cout << "LWR name: " << m_model->GetName() << " childCount=" << m_model->GetChildCount() << std::endl;
    std::cout << "Children: " << std::endl;
    for (unsigned childIdx = 0; childIdx < m_model->GetChildCount(); childIdx++) {
      physics::BasePtr child = m_model->GetChild(childIdx);
      std::cout << childIdx << " name=" << child->GetName();
      std::cout << std::endl;
    }
#endif 

    // Extract only joints belonging to current lwr, even if it is part of larger model
    physics::Joint_V joints = m_model->GetJoints();
    std::cout << lwrName << " joints:" << std::endl;
    for (size_t jointIdx = 0; jointIdx < joints.size(); jointIdx++) {
      physics::JointPtr currJoint = joints[jointIdx];
      if (lwrName == currJoint->GetName().substr(0, lwrNameLen) || currJoint->GetName().find(std::string("::") + lwrName) != std::string::npos) {
        m_joints.push_back(currJoint);
        std::cout << jointIdx << " name=" << currJoint->GetName() << " angle=" << currJoint->GetAngle(0) << " v=" << currJoint->GetVelocity(0) << std::endl;
      }
    }

    // TODO
    // send tFriMsrData to m_sendFriPort (i.e. what friremote receives)
    // listen on m_recvFriPort for tFriCmdData (i.e. what friremote sends)
    m_sendUdpSocket = new QUdpSocket();
    m_sendUdpSocket->connectToHost(QHostAddress::LocalHost, m_sendFriPort);
    m_recvUdpSocket = new QUdpSocket();
    m_recvUdpSocket->bind(QHostAddress::LocalHost, m_recvFriPort);
    // communication starts with LWR (this plugin) sending first packet (otherwise see ICRACK-component.cpp)


    // Listen to the update event. This event is broadcast every
    // simulation iteration.
    m_updateConnection = event::Events::ConnectWorldUpdateBegin(
        boost::bind(&LwrModelFRIPlugin::OnUpdate, this));
  }

  void
  LwrModelFRIPlugin::OnUpdate()
  {
    ros::Duration sinceLastUpdateDuration = ros::Time::now() - m_lastUpdateTime;

    if (sinceLastUpdateDuration.toSec() > m_updatePeriod) {
      updateRobotState();
      publishRobotState();
    }

    m_lastUpdateTime = ros::Time::now();
  }
/*------------------------------------------------------------------------}}}-*/

/*---------------------------------- private: ----------------------------{{{-*/
  bool
  LwrModelFRIPlugin::loadParams(sdf::ElementPtr _sdf)
  {
    m_nodeName = _sdf->GetParent()->Get<std::string>("name");

    m_sendFriPort = ahb::string::toIntSlow<uint16_t>(_sdf->GetElement("sendFriPort")->Get<std::string>());
    m_recvFriPort = ahb::string::toIntSlow<uint16_t>(_sdf->GetElement("recvFriPort")->Get<std::string>());

    return true;
  }

  void
  LwrModelFRIPlugin::updateRobotState()
  {
  }

  void
  LwrModelFRIPlugin::publishRobotState()
  {
  }
/*------------------------------------------------------------------------}}}-*/


  // Register this plugin with the simulator
  GZ_REGISTER_MODEL_PLUGIN(LwrModelFRIPlugin)
}

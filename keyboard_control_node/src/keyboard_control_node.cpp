 #include <ros/ros.h>
 #include <geometry_msgs/Twist.h>
 #include <signal.h>
 #include <termios.h>
 #include <stdio.h>
 
 #define KEYCODE_R 0x43 
 #define KEYCODE_L 0x44
 #define KEYCODE_U 0x41
 #define KEYCODE_D 0x42
 #define KEYCODE_Q 0x71
 
 class OpKey
 {
 public:
   OpKey();
   void keyLoop();
 
 private:
   ros::NodeHandle nodeHandle;
   double linear, angular, l_scale, a_scale;
   ros::Publisher twist_pub_;
   
 };
 
 OpKey::OpKey():
   linear(0),
   angular(0),
   l_scale(2.0),
   a_scale(2.0)
 {
   nodeHandle.param("scale_angular", a_scale, a_scale);
   nodeHandle.param("scale_linear", l_scale, l_scale);
 
   twist_pub_ = nodeHandle.advertise<geometry_msgs::Twist>("cmd_vel", 1);
 }
 
 int kfd = 0;
 struct termios cooked, raw;
 
 void quit(int sig)
 {
   (void)sig;
   tcsetattr(kfd, TCSANOW, &cooked);
   ros::shutdown();
   exit(0);
 }
 
 int main(int argc, char** argv)
 {
   ros::init(argc, argv, "op");
   OpKey anOpKey;
 
   signal(SIGINT,quit);
 
   anOpKey.keyLoop();
   quit(0);
   
   return(0);
 }
 
 
 void OpKey::keyLoop()
 {
   char c;
   bool dirty=false;
 
 
   // get the console in raw mode                                                              
   tcgetattr(kfd, &cooked);
   memcpy(&raw, &cooked, sizeof(struct termios));
   raw.c_lflag &=~ (ICANON | ECHO);
   // Setting a new line, then end of file                         
   raw.c_cc[VEOL] = 1;
   raw.c_cc[VEOF] = 2;
   tcsetattr(kfd, TCSANOW, &raw);
 
   puts("Reading from keyboard");
   puts("---------------------------");
   puts("Use arrow keys to move the robot. 'q' to quit.");
 
 
   for(;;)
   {
     // get the next event from the keyboard  
     if(read(kfd, &c, 1) < 0)
     {
       perror("read():");
       exit(-1);
     }
 
     linear = 0;
     angular = 0;
     ROS_DEBUG("value: 0x%02X\n", c);
   
     switch(c)
     {
       case KEYCODE_L:
         ROS_DEBUG("LEFT");
         angular = 1.0;
         dirty = true;
         break;
       case KEYCODE_R:
         ROS_DEBUG("RIGHT");
         angular = -1.0;
         dirty = true;
         break;
       case KEYCODE_U:
         ROS_DEBUG("UP");
         linear = 1.0;
         dirty = true;
         break;
       case KEYCODE_D:
         ROS_DEBUG("DOWN");
         linear = -1.0;
         dirty = true;
         break;
       case KEYCODE_Q:
         ROS_DEBUG("quit");
         return;
     }
    
 
     geometry_msgs::Twist twist;
     twist.angular.z = a_scale * angular;
     twist.linear.x = l_scale * linear;
     if(dirty ==true)
     {
       twist_pub_.publish(twist);    
       dirty=false;
     }
   }
 
   return;
 }

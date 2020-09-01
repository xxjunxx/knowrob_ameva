# knowrob_ameva
Only tested on Ubuntu 18.04
## Knowrob installation
### required dependency
1. install ROS-Melodic https://wiki.ros.org/melodic/Installation/Ubuntu
2. install mongodb https://docs.mongodb.com/manual/tutorial/install-mongodb-on-ubuntu/
3. install swi-prolog https://www.swi-prolog.org/build/PPA.html
4. install mongo-c-driver http://mongoc.org/libmongoc/current/installing.html

### build knowrob project
```
mkdir catkin_ws
cd catkin_ws
rosdep update
wstool init src
cd src
wstool merge https://raw.github.com/knowrob/knowrob/master/rosinstall/knowrob-base.rosinstall
wstool update
rosdep install --ignore-src --from-paths .
cd ~/catkin_ws
catkin_make 
```

## Launch knowrob
### Terminal One
1. make sure mongodbd is running
2. run following commands
```
cd catkin_ws
source devel/setup.bash
roslaunch knowrob knowrob.launch
```

### Terminal Two
1. Two ways to start swi-prolog. Personally prefer the first way.

 `rosrun rosprolog rosprolog_commandline.py` 
 `rosrun rosprolog rosprolog`
2.launch swi-prolog and play with pre-defined predicate
 
```
cd catkin_ws
source devel/setup.bash
rosrun rosprolog rosprolog_commandline.py 
// or we can use this 
// should be able to use the prolog
tripledb_load('')  -  for parsing owl file, probably need to use absolute path
triple_tell(s,p,o) - It's similar to rdf_has or 
owl_has

 ```

## Launch Knowrob
### Install dependency
1. Install libwebsocket https://stackoverflow.com/questions/29470447/how-to-install-libwebsocket-library-in-linux
`sudo apt-get install libwebsockets-dev`
2. Install protobuf
https://gist.github.com/diegopacheco/cd795d36e6ebcd2537cd18174865887b

### Run knowrob
1. Clone project in catkin_ws/src
2. build the package `catkin_make knowrob_ameve`
3. Two ways to load the package
 ```
//Method one:
 rosrun rosprolog rosprolog_commandline.py
 register_ros_package('knowrob').
//Method two:
rosrun rosprolog rosprolog knowrob_ameva
 ```



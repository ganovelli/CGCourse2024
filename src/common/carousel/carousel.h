#pragma once

#include <vector>
#include <time.h> 
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>
#include "..\box3.h"

struct carousel_loader;
class race;

struct point_object {
	point_object():pos(0.f) {}
	glm::vec3 pos;
};


struct stick_object :public point_object{
	stick_object():height(0){};
	float height;
};

/** @name Carousel
 *  
*/
//@{


 
/** Cameramen
	The cameraman films the race from a given specified point of view. 
	The cameraman has a  radius of action. When a car is within is radius the cameraman points the camera to it.
	The cameraman is represented with a 4x4 matrix encoding its view frame
*/
struct cameraman {
	friend race;
	cameraman(float r) :radius(r), locked(false) {}

	/// cameraman view reference frame
	glm::mat4 frame;

private:
	/// lock if a car is closer than radius
	float radius;

	/// which car is looking at
	int target_car;
	
	/// is looking to  a car
	bool locked;
};
 
 
/**
	 the track is described by its left and right curbs, when a curb is a sequence of n 3d points describing a closed line 
	 (so   point n-1 is connected to point 0 ). The two curbs have the same number of points and can be draw with a 
	 GL_TRIANGLE_STRIP providing vertices as curb[0][0],curb[1][0],curb[0][1],curb[1][1] and so on...
*/
struct track {
	friend race;
	track() {}
	std::vector<glm::vec3> curbs[2]; 

	///length of the track
	float length;

private:
	void compute_length() {
		length = 0.f;
		for (size_t i = 0; i < curbs[0].size();++i)  
			length += glm::length(curbs[0][(i + 1) % curbs[0].size()] - curbs[0][i]);
	}
};

struct path {

	// store one frame every 1/30 of second
	// frames[i] is the frame at time i* (1000.f/30.0) in milliseconds
	std::vector<glm::mat4> frames; 

	int T; // how many milliseconds for a lap to complete
};


 
/**
	  a car is described by its frame. It is intended that the wheels of the car are on the plane XZ and its front is 
	  toward -Z. A box of the car is also provided.
*/
struct car {
	friend race;

	/// local frame of the car. the car front is on -z halfspace
	glm::mat4 frame;

	/// box is the bounding box of the car, expressed in "frame" coordinates
	box3 box;
	 
private:
	/// on which path is the car moving
	int id_path;

	/// starting point
	int delta_i;
};

 
/**
	   the terrain is a height field, that is, an height value (y) for each (x,z) coordinates.
*/
struct  terrain {
	
	/// terrain specified as an height field
	std::vector<unsigned char> height_field;

	/// rectangle in xz where the terrain is located (minx,miny,sizex,sizey)
	glm::vec4 rect_xz;

	/// size in pixels of the height field image
	glm::ivec2 size_pix;

	
	float  hf(const unsigned int i, const unsigned int j) const {
		return height_field[ (size_pix[0]-1-i) * size_pix[0] + j]/50.f;
	}

 
	/// given a 3D point, returns its orthogonal projection into the terrain (that is, along the y direction)
	glm::vec3  p(glm::vec3 p_in) {
		return glm::vec3(p_in.x, y(p_in.x, p_in.z), p_in.z);
	}

	/// given the x and z coordinates, returns the height of the terrain in (x,z)
	float y(float x, float z) const {
		float yy = (z - rect_xz[1]) ;
		float xx = (x - rect_xz[0]) ;
		float sx = rect_xz[2] / size_pix[0];
		float sy = rect_xz[3] / size_pix[1];

		float i_min = xx / sx;
		float j_min = yy / sy;

		int i = static_cast<int>(floor(i_min));
		int j = static_cast<int>(floor(j_min));

		float u = i_min - i;
		float v = j_min - j;

		float value =	hf(i	, j		) * (1.f - u) * (1.f - v) +
						hf(i	, j + 1 ) * (1.f - u) * v +
						hf(i + 1, j		) * u * (1.f - v) +
						hf(i + 1, j + 1 ) * u * v;
		return	value;
	}

};




 
/**
	   a race is a scene with some static (terrain, trees, lamps...) and some dynamic components (cars, cameramen, sunlight direction) 
*/
class race {
	friend carousel_loader;
public:
	race():sim_time_ratio(60){}

	/// bounding box of the whole scene
	const box3& bbox() const {	return _bbox;}

	/// terrain height field
	const terrain& ter() const { return _ter; }

	/// race track
	const track& t() const { return _t; }

	/// sunlight direction. It changes with time
	const glm::vec3& sunlight_direction() const { return _sunlight_direction; }

	/// a vector of trees
	const std::vector<stick_object> & trees() const { return _trees;    }

	/// a vector of lamps
	const std::vector<stick_object> & lamps() const { return _lamps;    }

	/// a vector of cameramen
	const std::vector<cameraman>& cameramen() const { return _cameramen;}

	/// a vector of cars
	const std::vector<car>  &          cars() const { return _cars;     }
	 
private:
	box3 _bbox;
	terrain _ter;
	track _t;
	glm::vec3 _sunlight_direction;
	std::vector<stick_object> _trees;
	std::vector<stick_object> _lamps;
	std::vector<cameraman> _cameramen;
	std::vector<car> _cars;


	std::vector<path> carpaths;

	int clock_start;

	/// simulation sunlight time in milliseconds
	int sim_time;

	/// how long a real second in simulated sunlight time
	int sim_time_ratio; 

public:
	/**   
	 * starts the carousel
	 * @param hour  
	 * @param minute  
	 * @param second  
	 * @param ratio between the actual time and the simulated time for the sunlight direction
	 */
	void start( int h = -1, int m = -1, int s = -1, int _sim_time_ratio = 60) {
		clock_start = clock();
		if (h != -1) 
			sim_time = (s + m * 60 + h * 3600) * 1000;
		else
			sim_time = ( 10 * 3600) * 1000; // start at ten in the morning
		if (_sim_time_ratio != 60)
			sim_time_ratio = _sim_time_ratio;
	}

	/**   
	* add a car to the carousel
	* @param id_path add it to a specific path
	* @param delta shift the starting point along the path
	* */
	void add_car(int id_path, float delta = -1) {
		if (id_path >= carpaths.size()) {
			std::cout << "car path > " << carpaths.size() - 1 << "\n";
			exit(-1);
		}
		car c; 
		c.box.add(glm::vec3(-1, 1.5, -2));
		c.box.add(glm::vec3( 1, 0,    2));
		c.id_path = id_path;
		 
		c.delta_i = (int) floor(((delta == -1) ? rand() / float(RAND_MAX):delta) * (carpaths[id_path].frames.size() - 2));

		_cars.push_back(c);
	}

	/**
	 * add a car to the carousel.
	 * 
	 * @param delta shift the starting point along the path
	 */
	void add_car(float delta = -1) {
		int id = (int) floor((rand() / float(RAND_MAX)) *  carpaths.size());
		add_car(id,delta);
	}

	/**
	 * update the carousel. Call this at the beginning of any render cycle
	 * */
	void update() {
		int cs = clock() - clock_start;
		for (size_t i = 0; i < _cars.size();++i) {
			int ii = ((int)((cs) / 1000.f * 30.f)+ _cars[i].delta_i) % carpaths[_cars[i].id_path].frames.size();
			//std::cout << ii << std::endl;
			_cars[i].frame = carpaths[_cars[i].id_path].frames[ii];
		}
		int day_ms = 3600000 * 24;
		 
		int daytime = (  this->sim_time + cs * sim_time_ratio) % (day_ms);
		glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(360.f * daytime / float(day_ms)), glm::vec3(1, 0, 0));
		_sunlight_direction = R * glm::vec4(0.f, -1.f, 0.f,0.f);

		// update cameramen frames
		for (unsigned int ic = 0; ic < _cameramen.size(); ++ic) {
			cameraman& c = _cameramen[ic];
			glm::vec3 cp = *(glm::vec3*)&c.frame[3];

			if (!c.locked) {
				for (unsigned int ica = 0; ica < _cars.size(); ++ica)
					if (glm::length(cp - *(glm::vec3*)&_cars[ica].frame[3]) < c.radius) {
						c.target_car = ica;
						c.locked = true;
						break;
					}
			}

			if (c.locked) {
				glm::vec3 tcp = *(glm::vec3*)&_cars[c.target_car].frame[3];
				if (glm::length(cp - tcp) < c.radius)
				{
					c.frame = glm::lookAt(cp, tcp, glm::vec3(0, 1, 0));
					c.frame = glm::inverse(c.frame);
				}
				else
					c.locked = false;
			}
		}
	}

};


//@}

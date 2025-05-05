#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <vector>
#include <mutex>
#include <atomic>


#include "common.hpp"
#include "glm.hpp"

using namespace glm;

namespace glimac {

    enum ParticuleComputeType {
        fixed = 0u,
        euler = 1u,
        leapfrog = 2u
    };

    class Particule {

        public:

            float m_mass;
            vec3 m_pos;
            vec3 m_speed;
            // vec3 m_accel
            vec3 m_forces_acc;

            Particule(float mass, vec3 pos, vec3 speed, ParticuleComputeType type) {
                m_mass = mass;
                m_pos = pos;
                m_speed = speed;
                m_forces_acc = vec3(0);
                m_type = type;

                m_initial_pos = m_pos;
                m_initial_speed = m_speed;

                // std::cerr << "Creating the Mutex" << std::endl;
                m_mutex = new std::mutex();
                // std::cerr << "Mutex OK" << std::endl;
            }

            Particule(float mass, vec3 pos, ParticuleComputeType type):Particule(mass, pos, vec3(0), type){}

            Particule(float mass, vec3 pos) : Particule(mass, pos, fixed) {
            }

            ~Particule(){
                delete m_mutex;
            }

            void update(float h) {
                std::unique_lock<std::mutex> guard(*m_mutex);
                // std::lock_guard<std::mutex> guard(m_mutex);
                // m_mutex.lock();
                switch (m_type)
                {
                case ParticuleComputeType::euler:
                    update_euler(h);
                    break;
                case ParticuleComputeType::leapfrog:
                    update_leapfrog(h);
                    break;
                default:
                    update_fixed(h);
                    break;
                }
                // m_mutex.unlock();
            }

            void reset() {
                std::unique_lock<std::mutex> guard(*m_mutex);
                // std::lock_guard<std::mutex> guard(m_mutex);
                // m_mutex.lock();
                m_pos = m_initial_pos;
                m_speed = m_initial_speed;
                // m_mutex.unlock();
            }

            void setType(ParticuleComputeType type) {
                std::unique_lock<std::mutex> guard(*m_mutex);
                // std::lock_guard<std::mutex> guard(m_mutex);
                // m_mutex.lock();
                m_type = type;
                if(m_type == ParticuleComputeType::fixed) {
                    m_speed = vec3(0);
                }
                // m_mutex.unlock();
            }

            bool try_lock() {
                return m_mutex->try_lock();
                // return m_mutex.try_lock();
            }

            void unlock() {
                m_mutex->unlock();
                // m_mutex.unlock();
            }

            
        private:
            ParticuleComputeType m_type;

            vec3 m_initial_pos;
            vec3 m_initial_speed;

            std::mutex* m_mutex;

            void update_leapfrog(float h) {
                m_speed += h * m_forces_acc / m_mass;
                m_pos += h * m_speed;
                m_forces_acc = vec3(0);
            }
                
            void update_euler(float h) {
                m_pos += h * m_speed;
                m_speed += h * m_forces_acc / m_mass;
                m_forces_acc = vec3(0);
            }
                
            void update_fixed(float /*h*/) {
                m_forces_acc = vec3(0);
            }

    };

}
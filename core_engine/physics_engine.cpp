/*
Bubble Shooter Physics Engine - C++ Core
High-performance physics and collision detection
Compiled to WebAssembly for browser use
*/

#include <cmath>
#include <vector>
#include <algorithm>
#include <emscripten/bind.h>

constexpr float PI = 3.14159265358979323846f;

// Vector2D structure for positions
struct Vector2D {
    float x, y;
    
    Vector2D(float x = 0, float y = 0) : x(x), y(y) {}
    
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }
    
    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }
    
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    float distanceTo(const Vector2D& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    Vector2D normalized() const {
        float len = length();
        if (len > 0) {
            return Vector2D(x / len, y / len);
        }
        return Vector2D(0, 0);
    }
};

// Game Object Base Class
class GameObject {
public:
    Vector2D position;
    Vector2D velocity;
    float radius;
    bool active;
    
    GameObject(float x = 0, float y = 0, float radius = 10)
        : position(x, y), velocity(0, 0), radius(radius), active(true) {}
    
    virtual void update(float deltaTime) {
        position.x += velocity.x * deltaTime;
        position.y += velocity.y * deltaTime;
    }
    
    virtual ~GameObject() = default;
};

// Bubble Class
class Bubble : public GameObject {
public:
    int colorIndex;
    float speed;
    bool isSpecial;
    
    Bubble(float x, float y, float radius, float speed, int colorIndex = 0)
        : GameObject(x, y, radius), speed(speed), colorIndex(colorIndex), isSpecial(false) {
        velocity.y = speed;  // Falling down
    }
    
    void update(float deltaTime) override {
        // Apply gravity-like acceleration
        velocity.y += 0.1f * deltaTime * speed;
        
        // Add slight horizontal movement for variety
        velocity.x = std::sin(position.y * 0.01f) * 0.5f;
        
        GameObject::update(deltaTime);
    }
};

// Bullet Class
class Bullet : public GameObject {
public:
    float speed;
    float maxDistance;
    float distanceTraveled;
    
    Bullet(float x, float y, float angle, float speed = 8.0f)
        : GameObject(x, y, 5), speed(speed), maxDistance(1000), distanceTraveled(0) {
        velocity.x = std::cos(angle) * speed;
        velocity.y = -std::sin(angle) * speed;  // Shooting upward
    }
    
    void update(float deltaTime) override {
        GameObject::update(deltaTime);
        distanceTraveled += std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y) * deltaTime;
        
        // Deactivate if traveled too far
        if (distanceTraveled > maxDistance) {
            active = false;
        }
    }
};

// Main Physics Engine
class PhysicsEngine {
private:
    std::vector<Bubble> bubbles;
    std::vector<Bullet> bullets;
    float gameTime;
    int bubblesDestroyed;
    float speedMultiplier;
    
public:
    PhysicsEngine() : gameTime(0), bubblesDestroyed(0), speedMultiplier(1.0f) {}
    
    // Initialize with bubbles
    void initialize(int bubbleCount, float canvasWidth) {
        bubbles.clear();
        bullets.clear();
        
        for (int i = 0; i < bubbleCount; ++i) {
            float radius = 12.0f + static_cast<float>(rand()) / RAND_MAX * 8.0f;
            float x = radius + static_cast<float>(rand()) / RAND_MAX * (canvasWidth - 2 * radius);
            float y = -radius - static_cast<float>(rand()) / RAND_MAX * 100.0f;
            float speed = 1.0f + static_cast<float>(rand()) / RAND_MAX * 1.5f;
            int color = rand() % 5;
            
            bubbles.emplace_back(x, y, radius, speed, color);
        }
        
        gameTime = 0;
        bubblesDestroyed = 0;
    }
    
    // Update all game objects
    void update(float deltaTime) {
        gameTime += deltaTime;
        
        // Update bubbles
        for (auto& bubble : bubbles) {
            if (bubble.active) {
                bubble.update(deltaTime);
            }
        }
        
        // Update bullets
        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.update(deltaTime);
            }
        }
        
        // Check collisions
        checkCollisions();
        
        // Remove inactive objects
        cleanup();
    }
    
    // Add a new bubble
    void addBubble(float x, float y, float radius, float speed, int color) {
        bubbles.emplace_back(x, y, radius, speed, color);
    }
    
    // Shoot a bullet
    void shootBullet(float x, float y, float angle) {
        bullets.emplace_back(x, y, angle);
    }
    
    // Check collisions between bullets and bubbles
    int checkCollisions() {
        int collisions = 0;
        
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            
            for (auto& bubble : bubbles) {
                if (!bubble.active) continue;
                
                float dx = bullet.position.x - bubble.position.x;
                float dy = bullet.position.y - bubble.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance < bullet.radius + bubble.radius) {
                    // Collision detected
                    bullet.active = false;
                    bubble.active = false;
                    collisions++;
                    bubblesDestroyed++;
                    
                    // Create explosion effect (simulated by smaller bubbles)
                    createExplosion(bubble.position.x, bubble.position.y, bubble.radius);
                    break;
                }
            }
        }
        
        return collisions;
    }
    
    // Create explosion effect
    void createExplosion(float x, float y, float radius) {
        int particleCount = 6 + rand() % 4;
        for (int i = 0; i < particleCount; ++i) {
            float angle = 2.0f * PI * i / particleCount;
            float speed = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;
            float particleRadius = radius * 0.3f;
            
            Bubble particle(x, y, particleRadius, speed);
            particle.velocity.x = std::cos(angle) * speed;
            particle.velocity.y = std::sin(angle) * speed;
            particle.colorIndex = 4; // Yellow for explosion
            
            // Particles fade quickly
            bubbles.push_back(particle);
        }
    }
    
    // Clean up inactive objects
    void cleanup() {
        // Remove inactive bubbles
        bubbles.erase(
            std::remove_if(bubbles.begin(), bubbles.end(),
                [](const Bubble& b) { return !b.active; }),
            bubbles.end()
        );
        
        // Remove inactive bullets
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.active; }),
            bullets.end()
        );
    }
    
    // Set game speed
    void setSpeedMultiplier(float multiplier) {
        speedMultiplier = multiplier;
        for (auto& bubble : bubbles) {
            bubble.speed *= multiplier;
            bubble.velocity.y = bubble.speed;
        }
    }
    
    // Get bubble data for rendering
    emscripten::val getBubbleData() {
        emscripten::val result = emscripten::val::array();
        int index = 0;
        
        for (const auto& bubble : bubbles) {
            if (bubble.active) {
                emscripten::val bubbleData = emscripten::val::object();
                bubbleData.set("x", bubble.position.x);
                bubbleData.set("y", bubble.position.y);
                bubbleData.set("radius", bubble.radius);
                bubbleData.set("color", bubble.colorIndex);
                bubbleData.set("speed", bubble.speed);
                result.set(index++, bubbleData);
            }
        }
        
        return result;
    }
    
    // Get bullet data for rendering
    emscripten::val getBulletData() {
        emscripten::val result = emscripten::val::array();
        int index = 0;
        
        for (const auto& bullet : bullets) {
            if (bullet.active) {
                emscripten::val bulletData = emscripten::val::object();
                bulletData.set("x", bullet.position.x);
                bulletData.set("y", bullet.position.y);
                bulletData.set("radius", bullet.radius);
                result.set(index++, bulletData);
            }
        }
        
        return result;
    }
    
    // Get bubbles that reached bottom
    emscripten::val getBubblesAtBottom(float canvasHeight) {
        emscripten::val result = emscripten::val::array();
        int index = 0;
        
        for (const auto& bubble : bubbles) {
            if (bubble.active && bubble.position.y > canvasHeight - 10) {
                emscripten::val bubbleData = emscripten::val::object();
                bubbleData.set("x", bubble.position.x);
                bubbleData.set("y", bubble.position.y);
                bubbleData.set("radius", bubble.radius);
                result.set(index++, bubbleData);
            }
        }
        
        return result;
    }
    
    // Get statistics
    emscripten::val getStats() {
        emscripten::val stats = emscripten::val::object();
        stats.set("bubblesCount", (int)bubbles.size());
        stats.set("bulletsCount", (int)bullets.size());
        stats.set("bubblesDestroyed", bubblesDestroyed);
        stats.set("gameTime", gameTime);
        return stats;
    }
    
    // Check if any bubble reached bottom
    bool hasBubbleReachedBottom(float canvasHeight) {
        for (const auto& bubble : bubbles) {
            if (bubble.active && bubble.position.y > canvasHeight - 10) {
                return true;
            }
        }
        return false;
    }
    
    // Predict bullet trajectory (for AI)
    emscripten::val predictTrajectory(float startX, float startY, float angle, float speed, int steps) {
        emscripten::val trajectory = emscripten::val::array();
        
        float x = startX;
        float y = startY;
        float vx = std::cos(angle) * speed;
        float vy = -std::sin(angle) * speed;  // Negative because up is negative Y
        
        for (int i = 0; i < steps; ++i) {
            emscripten::val point = emscripten::val::object();
            point.set("x", x);
            point.set("y", y);
            trajectory.set(i, point);
            
            x += vx * 0.016f;  // Assuming 60 FPS
            y += vy * 0.016f;
        }
        
        return trajectory;
    }
};

// Export to JavaScript using Embind
EMSCRIPTEN_BINDINGS(physics_engine) {
    emscripten::class_<PhysicsEngine>("PhysicsEngine")
        .constructor<>()
        .function("initialize", &PhysicsEngine::initialize)
        .function("update", &PhysicsEngine::update)
        .function("addBubble", &PhysicsEngine::addBubble)
        .function("shootBullet", &PhysicsEngine::shootBullet)
        .function("checkCollisions", &PhysicsEngine::checkCollisions)
        .function("setSpeedMultiplier", &PhysicsEngine::setSpeedMultiplier)
        .function("getBubbleData", &PhysicsEngine::getBubbleData)
        .function("getBulletData", &PhysicsEngine::getBulletData)
        .function("getBubblesAtBottom", &PhysicsEngine::getBubblesAtBottom)
        .function("getStats", &PhysicsEngine::getStats)
        .function("hasBubbleReachedBottom", &PhysicsEngine::hasBubbleReachedBottom)
        .function("predictTrajectory", &PhysicsEngine::predictTrajectory);
    
    emscripten::value_array<Vector2D>("Vector2D")
        .element(&Vector2D::x)
        .element(&Vector2D::y);
}

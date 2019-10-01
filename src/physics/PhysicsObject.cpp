#include "PhysicsObject.h"

bool inRange(float n, float low, float high)
{
    return low <= n && n <= high;
}

PhysicsObject::PhysicsObject(vec3 position, shared_ptr<Shape> model, shared_ptr<Collider> collider) : PhysicsObject::PhysicsObject(position, quat(1, 0, 0, 0), vec3(1, 1, 1), model, collider) {}

PhysicsObject::PhysicsObject(vec3 position, quat orientation, shared_ptr<Shape> model, shared_ptr<Collider> collider) : PhysicsObject::PhysicsObject(position, orientation, vec3(1, 1, 1), model, collider) {}

PhysicsObject::PhysicsObject(vec3 position, quat orientation, vec3 scale, shared_ptr<Shape> model, shared_ptr<Collider> collider) : GameObject(position, orientation, scale, model)
{
    if (collider != nullptr) this->collider = collider;
    else this->collider = make_shared<ColliderMesh>(model);

    this->netForce = vec3(0, 0, 0);
    this->impulse = vec3(0, 0, 0);
    this->acceleration = vec3(0, 0, 0);
    this->velocity = vec3(0, 0, 0);
    this->mass = 0;
    this->invMass = 0;
    this->normForce = vec3(0, 0, 0);
    this->friction = 0;
    this->elasticity = 0;
    this->speed = 0;
    this->ignoreCollision = false;
    this->solid = true;
}

void PhysicsObject::update()
{
    normForce = vec3(0);
    netForce.y += GRAVITY * mass;

    // filter collisions so that objects don't bump over edges
    vector<Collision *> faceCollisions;
    vector<Collision *> notFaceCollisions;
    for (int i = 0; i < collider->pendingCollisions.size(); i++)
    {
        switch (collider->pendingCollisions[i].geom)
        {
            case FACE:
                faceCollisions.push_back(&collider->pendingCollisions[i]);
                break;
            case EDGE:
            case VERT:
                notFaceCollisions.push_back(&collider->pendingCollisions[i]);
                break;
        }
    }
    unordered_set<Collision *> collisionsToRemove;
    for (int i = 0; i < faceCollisions.size(); i++)
    {
        for (int j = 0; j < notFaceCollisions.size(); j++)
        {
            if (faceCollisions[i]->other != notFaceCollisions[j]->other)
            {
                float d;
                intersectRayPlane(notFaceCollisions[j]->pos, -faceCollisions[i]->normal,
                    faceCollisions[i]->v[0], faceCollisions[i]->normal, d);
                if (d < 0.1)
                {
                    collisionsToRemove.insert(notFaceCollisions[j]);
                }
            }
        }
    }
    for (size_t i = collider->pendingCollisions.size() - 1; i >= 0 && !collisionsToRemove.empty(); i--)
    {
        Collision *col = &collider->pendingCollisions[i];
        if (collisionsToRemove.find(col) != collisionsToRemove.end())
        {
            collisionsToRemove.erase(col);
            collider->pendingCollisions.erase(collider->pendingCollisions.begin() + i);
        }
    }

    float maxImpact = -1;
    Collision maxImpactCollision;
    for (Collision collision : collider->pendingCollisions)
    {
        // resolve collision
        PhysicsObject *other = collision.other;

        if (!solid || !other->solid) continue;

        vec3 relVel = other->velocity - velocity;
        float velAlongNormal = dot(relVel, collision.normal);
        if (velAlongNormal < 0)
        {
            float e = (std::min)(other->elasticity, elasticity);
            float j = (-(1 + e) * velAlongNormal) / (invMass + other->invMass);
            vec3 colImpulse = j * collision.normal;
            velocity -= invMass * colImpulse;

            // normal force
            vec3 localNormForce = collision.normal * dot(netForce, -collision.normal);
            normForce += localNormForce;

            // friction
            vec3 frictionDir = (relVel - proj(relVel, collision.normal));
            float frictionLen = length(frictionDir);
            if (frictionLen > 0)
            {
                if (frictionLen > 0.1)
                {
                    frictionDir = normalize(frictionDir);
                }
                vec3 frictionForce = length(localNormForce) * friction * frictionDir;
                if (!isnan(frictionForce.x))
                {
                    netForce += frictionForce;
                }
            }

            // correct position to prevent sinking/jitter
            if (other->invMass == 0)
            {
                float percent = 0.2f;
                float slop = 0.01f;
                vec3 correction = (std::max)(collision.penetration - slop, 0.0f) / (invMass + other->invMass) * percent * -collision.normal;
                position += invMass * correction;
            }

            if (fabs(velAlongNormal) > maxImpact)
            {
                maxImpact = fabs(velAlongNormal);
                maxImpactCollision = collision;
            }
        }
    }
    if (maxImpact > 0)
    {
        onHardCollision(maxImpact, maxImpactCollision);
    }
    clearCollisions();

    netForce += normForce;

    velocity += impulse * invMass;

    // drag
    if (velocity != vec3(0))
    {
        netForce += dot(velocity, velocity) * -normalize(velocity) * DRAG_COEFFICIENT;
    }

    // apply force
    acceleration = netForce * invMass;
    velocity += acceleration * Time.physicsDeltaTime;
    if (fabs(velocity.x) > 0.01)
    {
        position.x += velocity.x * Time.physicsDeltaTime;
    }
    if (fabs(velocity.y) > 0.01)
    {
        position.y += velocity.y * Time.physicsDeltaTime;
    }
    if (fabs(velocity.z) > 0.01)
    {
        position.z += velocity.z * Time.physicsDeltaTime;
    }

    impulse = vec3(0);
    netForce = vec3(0);
}

void PhysicsObject::start()
{

}

void PhysicsObject::lateUpdate()
{

}

void PhysicsObject::physicsUpdate()
{

}

void PhysicsObject::latePhysicsUpdate()
{

}

void PhysicsObject::checkCollision(PhysicsObject *other)
{
    if (other->collider != NULL && collider != NULL && !other->ignoreCollision && !ignoreCollision)
    {
        collider->checkCollision(this, other, other->collider.get());
    }
}

float PhysicsObject::getRadius()
{
    if (collider == NULL)
    {
        return 0;
    }
    else if (scale == vec3(1))
    {
        return collider->bbox.radius;
    }
    else
    {
        return collider->getRadius(scale);
    }
}

vec3 PhysicsObject::getCenterPos()
{
    if (collider == NULL || collider->bbox.center == vec3(0))
    {
        return position;
    }
    else if (orientation == quat(1, 0, 0, 0))
    {
        return position + collider->bbox.center * scale;
    }
    else
    {
        return position + vec3(mat4_cast(orientation) * vec4(collider->bbox.center * scale, 1));
    }
}

// Called when the object hits something with a normal velocity > 0
void PhysicsObject::onHardCollision(float impactVel, Collision &collision)
{

}

void PhysicsObject::applyImpulse(vec3 impulse)
{
    this->impulse += impulse;
}

void PhysicsObject::setMass(float mass)
{
    this->mass = mass;
    if (mass == 0) this->invMass = 0;
    else this->invMass = 1.0f / mass;
}

void PhysicsObject::setFriction(float friction)
{
    this->friction = friction;
}

void PhysicsObject::setElasticity(float elasticity)
{
    this->elasticity = elasticity;
}

void PhysicsObject::setVelocity(vec3 velocity)
{
    this->velocity = velocity;
}

vec3 PhysicsObject::getVelocity()
{
    return this->velocity;
}

void PhysicsObject::clearCollisions()
{
    if (collider != nullptr)
    {
        collider->clearCollisions(this);
    }
}
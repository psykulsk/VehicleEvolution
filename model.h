#ifndef MODEL_H
#define MODEL_H

#include <Box2D/Box2D.h>
#include "chromosome.h"


/*! Model is a wrapper for b2World. It isn't dependent on the View object.
 * \brief Model is a wrapper for b2World. It isn't dependent on the View object
 */
class Model
{
public:
    Model();

    /*!
     * \brief wrapper for Box2D simulation step.
     */
    void simulate();

    /*!
     * \brief Adds rectangular body to Box2D world.
     * \param posX x coordinate of the body center in Box2D world coordinate system.
     * \param posY y coordinate of the body center in Box2D world coordinate system.
     * \param bodyType type of body (static/dynamic/kinematic).
     * \param angle_radians initial body angle (in radians).
     * \return pointer to newly created body.
     */
    b2Body* addRectBody(float posX, float posY, b2BodyType bodyType, float angle_radians);

    void addTrack();

    b2Body* addWheelBody(float poxX, float posY);
    b2Fixture* addWheelFixture(b2Body* parentBody, float radius, float density, float friction, float restitution, uint16 collisionGroup);
    b2Fixture* addCircleFixture(b2Body *parentBody, float posX, float posY, float radius,
                                       float density, float friction, float restitution);

    b2Joint* addRevoluteJoint(b2RevoluteJointDef* revoluteJointDef);

    b2Body* addCarFromChromosome(Chromosome chromosome, float posX, float posY);

    void deleteCar();

    b2Body* chromosomeCarBodyPtr_;
    b2Body* groundBodyPtr_;
    std::array<b2Body*, Chromosome::NUMBER_OF_WHEELS> WheelBodyPtrArray;

private:
    /*!
     * \brief Creates ground chain shape which is a track used to evaluate cars.
     * \param parentBody
     * \param points
     * \param pointsCount
     * \param density
     * \param friction
     * \param restitution
     * \param collisionGroup
     * \return
     */
    b2Fixture* addGroundChainShape(b2Body *parentBody, b2Vec2 * points, unsigned int pointsCount,
                                       float density, float friction, float restitution, uint16 collisionGroup);

    static const float32 BOX2D_TIMESTEP;
    static const int32 BOX2D_VELOCITY_ITERATIONS;
    static const int32 BOX2D_POSITION_ITERATIONS;
    static const float WORLD_X_GRAVITY_VALUE;
    static const float WORLD_y_GRAVITY_VALUE;

    b2World box2dWorld_;
};

#endif // MODEL_H

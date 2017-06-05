#include "model.h"

const float32 Model::BOX2D_TIMESTEP = 1.0f/60.0f;
const int32 Model::BOX2D_VELOCITY_ITERATIONS = 6;
const int32 Model::BOX2D_POSITION_ITERATIONS = 2;
const float Model::WORLD_X_GRAVITY_VALUE = 0.0;
const float Model::WORLD_y_GRAVITY_VALUE = -10.0;
std::array<b2Body*, Chromosome::NUMBER_OF_WHEELS> WheelBodyPtrArray;

Model::Model()
    : box2dWorld_(b2Vec2(WORLD_X_GRAVITY_VALUE, WORLD_y_GRAVITY_VALUE))
{
}

b2Body* Model::addRectBody(float posX, float posY, b2BodyType bodyType, float angle_radians)
{
    b2BodyDef bodyDef;
    bodyDef.position.Set(posX,posY);
    bodyDef.type = bodyType;
    bodyDef.angle = angle_radians;

    return box2dWorld_.CreateBody(&bodyDef);
}

b2Fixture* Model::addGroundChainFixture(b2Body *parentBody, b2Vec2 * points, unsigned int pointsCount,
                                   float density, float friction, float restitution, uint16 collisionGroup)
{
    groundBodyPtr_ = parentBody;
    b2ChainShape chainShape;
    chainShape.CreateChain(points,pointsCount);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &chainShape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;
    fixtureDef.filter.groupIndex = collisionGroup; //Ustawiam collision group na -1 -> fixtures z tej grupy nie zderzają się ze sobą.
    return parentBody->CreateFixture(&fixtureDef);
}

void Model::addTrack()
{
    b2Body* testBody = addRectBody(-80.0f,0.0f,b2_staticBody, 0.0f);
    b2Vec2 points[500];
    float x = -150.0;
    // Set random number generation seed to 1, so the track appears as random, but doesn't change beetween instances
    std::default_random_engine trackGenerator;
    trackGenerator.seed(1);
    std::uniform_real_distribution<float> trackElevationDistribution(-15.0f,15.0f);


    for(int i=0; i<500; ++i){
        points[i].x = x;
        x += 40.0;
        float y;
        if( i > 7){
            y = trackElevationDistribution(trackGenerator);
        }else
        {
            y = 0.0;
        }

        points[i].y = y;
    }
    addGroundChainFixture(testBody, points, 100, 1.0f, 0.3f, 0.3f, 0);
}

b2Body* Model::addWheelBody(float posX, float posY)
{
    b2BodyDef bodyDef;
    bodyDef.position.Set(posX,posY);
    bodyDef.type = b2_dynamicBody;

    return box2dWorld_.CreateBody(&bodyDef);
}

b2Fixture* Model::addWheelFixture(b2Body* parentBody, float radius, float density, float friction, float restitution, uint16 collisionGroup)
{
    b2CircleShape circleShape;
    circleShape.m_radius = radius;
    circleShape.m_p.Set(0,0); // Position relative to parent body position

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;
    fixtureDef.filter.groupIndex = collisionGroup; //Ustawiam collision group na -1 -> fixtures z tej grupy nie zderzają się ze sobą.

    return parentBody->CreateFixture(&fixtureDef);
}

b2Joint* Model::addRevoluteJoint(b2RevoluteJointDef* revoluteJointDef)
{
    return box2dWorld_.CreateJoint(dynamic_cast<b2JointDef*>(revoluteJointDef));
}

b2Body* Model::addCarFromChromosome(Chromosome chromosome, float posX, float posY)
{
    //CAR BODY DEFINITION
    b2BodyDef carbodyDef;
    carbodyDef.position.Set(posX,posY);
    carbodyDef.type = b2_dynamicBody;
    //Create CAR FIXTURE DEFINITION STRUCT
    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0;
    fixtureDef.friction = 0.3;
    fixtureDef.restitution = 0.3;
    fixtureDef.filter.groupIndex = -2;
    //Create CAR BODY
    chromosomeCarBodyPtr_ = box2dWorld_.CreateBody(&carbodyDef);
    //CAR SHAPES
    b2Vec2 centerPoint(posX, posY);
    b2PolygonShape triangleShape;
    for(int i = 0; i < Chromosome::NUMBER_OF_VERTICES; ++i){
        triangleShape.Set(chromosome.CreateTriangleByIndexAndThridVertex(i,centerPoint).begin(),3);
        fixtureDef.shape = &triangleShape;
        chromosomeCarBodyPtr_->CreateFixture(&fixtureDef);
    }
    //Adding wheels
    int wheelBodyPtrIterator = 0;
    for(Wheel wheel : chromosome.getWheels())
    {
        //TODO znaleźć błąd -> dlaczego tutaj muszę mnożyć razy 2 a nie po prostu brać posX i posY
        //Axles
        // shape def
        b2PolygonShape axleShape;
        axleShape.SetAsBox(1,1);
        // fixture def
        b2FixtureDef axleFixture;
        axleFixture.density=0.5;
        axleFixture.friction=3;
        axleFixture.restitution=0.3;
        axleFixture.shape= &axleShape;
        axleFixture.filter.groupIndex=-2;
        // body def
        b2BodyDef axleBodyDef;
        axleBodyDef.type = b2_dynamicBody;
        axleBodyDef.position.Set(2*posX + wheel.wheelCenterPosition_.x, 2*posY + wheel.wheelCenterPosition_.y);
        // Actual body
        b2Body* axleBody = box2dWorld_.CreateBody(&axleBodyDef);
        // Actual Fixture
        axleBody->CreateFixture(&axleFixture);
        //wheel
        b2Body* wheelBody = addWheelBody(2*posX + wheel.wheelCenterPosition_.x, 2*posY + wheel.wheelCenterPosition_.y);
        addWheelFixture(wheelBody, wheel.wheelRadius_, 1.0, 0.3, 0.3, -2);
        //joint wheel to axle
        b2RevoluteJointDef wheelToAxleJointDef;
        wheelToAxleJointDef.Initialize(wheelBody,axleBody,wheelBody->GetWorldCenter());
        wheelToAxleJointDef.enableMotor=true;
        wheelToAxleJointDef.motorSpeed = 5;
        wheelToAxleJointDef.maxMotorTorque=1500;
        addRevoluteJoint(&wheelToAxleJointDef);
        //joint axle to car
        b2PrismaticJointDef axlePrismaticJointDef;
        axlePrismaticJointDef.lowerTranslation=-1;
        axlePrismaticJointDef.upperTranslation=1;
        axlePrismaticJointDef.enableLimit=true;
        axlePrismaticJointDef.enableMotor=true;
        //axle
        b2Vec2 axis(0,1);
        axlePrismaticJointDef.Initialize(chromosomeCarBodyPtr_, axleBody, axleBody->GetWorldCenter(), axis);
        box2dWorld_.CreateJoint(&axlePrismaticJointDef);

        WheelBodyPtrArray[wheelBodyPtrIterator] = wheelBody;
        ++wheelBodyPtrIterator;
    }

    return chromosomeCarBodyPtr_;
}

void Model::deleteCar()
{
    if(chromosomeCarBodyPtr_)
    {
        box2dWorld_.DestroyBody(chromosomeCarBodyPtr_);
        chromosomeCarBodyPtr_ = nullptr;
    }
}

void Model::simulate()
{
    box2dWorld_.Step(BOX2D_TIMESTEP, BOX2D_VELOCITY_ITERATIONS, BOX2D_POSITION_ITERATIONS);
}

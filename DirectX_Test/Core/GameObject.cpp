#include "GameObject.h"

void CGameObject::UpdateWorldMatrix()
{
	XMMATRIX Translation{ XMMatrixTranslationFromVector(ComponentTransform.Translation) };
	XMMATRIX Rotation{ XMMatrixRotationQuaternion(ComponentTransform.RotationQuaternion) };
	//XMMATRIX Rotation{ XMMatrixRotationAxis(ComponentTransform.Rotation) };
	XMMATRIX Scaling{ XMMatrixScalingFromVector(ComponentTransform.Scaling) };

	ComponentTransform.MatrixWorld = Scaling * Rotation * Translation;

	if (ComponentPhysics.isCollide) {
		XMFLOAT3 Point1 = ComponentPhysics.Point1;
		XMFLOAT3 Point2 = ComponentPhysics.Point2;
		XMStoreFloat3(&Point1, XMVector3Transform(XMLoadFloat3(&ComponentPhysics.Point1), ComponentTransform.MatrixWorld));
		XMStoreFloat3(&Point2, XMVector3Transform(XMLoadFloat3(&ComponentPhysics.Point2), ComponentTransform.MatrixWorld));
		ComponentPhysics.collisionBox.Center = { 
			(Point1.x + Point2.x) / 2, 
			(Point1.y + Point2.y) / 2, 
			(Point1.z + Point2.z) / 2, 
		};
		ComponentPhysics.collisionBox.Extents = {
			Point1.x - Point2.x > 0 ? (Point1.x - Point2.x) / 2 : (Point2.x - Point1.x) / 2,
			Point1.y - Point2.y > 0 ? (Point1.y - Point2.y) / 2 : (Point2.y - Point1.y) / 2,
			Point1.z - Point2.z > 0 ? (Point1.z - Point2.z) / 2 : (Point2.z - Point1.z) / 2,
		};
	}
}

void CGameObject::CreateCollision() 
{
	std::tuple<XMFLOAT3, XMFLOAT3> points = ComponentRender.PtrObject3D->GetBoxObject();// 1 < 2
	auto Points = [this](XMFLOAT3 p1, XMFLOAT3 p2){
		ComponentPhysics.Point1 = p1;
		ComponentPhysics.Point2 = p2;
	};
	std::apply(Points, points);
	ComponentPhysics.isCollide = true;
	UpdateWorldMatrix();
}
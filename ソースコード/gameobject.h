#pragma once
#include "main.h"
#include "easing.h"
#include <algorithm>
#include <limits>
#include <stdio.h>


class GameObject
{
protected:
	bool m_Destroy = false;

	Vector3 m_Position{ 0.f, 0.f, 0.f };
	Vector3 m_Rotation{ 0.f, 0.f, 0.f };
	Vector3 m_Scale{ 1.f, 1.f, 1.f };

public:
	virtual void Initialize() {};
	virtual void Update() {};
	virtual void Draw() {};
	virtual void Finalize() {};

	void setDestroy() { m_Destroy = true; };
	bool Destroy()
	{
		if (m_Destroy)
		{
			Finalize();
			delete this;
			return true;
		}
		else
		{
			return false;
		}
	};

	Vector3 GetPosition() { return m_Position; };
	void SetPosition(Vector3 position) { m_Position = position;};

	Vector3 GetRight() 
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		Vector3 right;
		XMStoreFloat3((XMFLOAT3*)&right, matrix.r[0]);
		return right;
	};

	Vector3 GetForward()
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(0.f, m_Rotation.y, m_Rotation.z);
		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, matrix.r[2]);
		return forward;
	};

	Vector3 GetRotationForward()
	{
		XMMATRIX matrix;
		matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, matrix.r[2]);
		return forward;
	};

	Vector3 GetRotation() { return m_Rotation; };
	void SetRotation(Vector3 rotation) { m_Rotation = rotation; };

	void SetRotationY(Vector3 rotation) 
	{
		rotation.z = 0.f;
		rotation.x = 0.f;
		m_Rotation = rotation;
	}

	Vector3 GetScale() { return m_Scale; };
	void SetScale(Vector3 scale) {m_Scale = scale; }

	

	void AddPosition(const Vector3& velocity) {
		m_Position += velocity;

	}

	

public:
	float GetDistance(Vector3 position)
	{
		Vector3 d = m_Position - position;
		return d.Length();
	};

	
	void SetLifeTime(float lifeTime)
	{
		static float timer = 0.f;
		timer += 1.f / 60.f;
		if (timer >= lifeTime)
		{
			setDestroy();
			timer = 0.f;
		}

	}

	float FloatLeap(float start, float end, float time)
	{
		return start + (end - start) * time;
	}

};

	
#pragma once  
class Camera;   
#include "gameobject.h"  

class Player : public GameObject  
{  
private:  
   ID3D11PixelShader* m_PixelShader;  
   ID3D11VertexShader* m_VertexShader;  
   ID3D11InputLayout* m_VertexLayout;  

   float m_MoveSpeed;
   float m_DashSpeed;

public:  
   void Initialize();  
   void Update();  
   void Draw();  
   void Finalize();  

   void PlayerAction(Camera* camera, Vector3 rotation);  
 
};
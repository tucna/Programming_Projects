#include <array>
#include <iomanip>
#include <sstream>

#define T_PGE_APPLICATION
#include "../engine/tPixelGameEngine.h"

#define PI 3.14159265358979323846f

using namespace std;

namespace g
{
  constexpr uint32_t screenWidth = 600;
  constexpr uint32_t screenHeight = 380;
};

using float4x4 = array<array<float, 4>, 4>;

struct float4 { float x, y, z, w; };
struct float3 { float x, y, z; };
struct float2 { float x, y; };

// Utils methods
float toRad(float deg) { return deg * PI / 180.0f; }
float dot(const float4& v1, const float4& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
float dot(const float3& v1, const float3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

float3 cross(const float3& v1, const float3& v2)
{
  return { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };
}

float3 normalize(const float3& v1)
{
  float length = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);

  float3 normalized =
  {
    v1.x / length,
    v1.y / length,
    v1.z / length,
  };

  return normalized;
}

// Operator overloading

float3 operator-(const float3 &v1) { return { -v1.x, -v1.y, -v1.z }; }
float3 operator-(const float3 &v1, const float3 &v2)
{
  float3 difference =
  {
    v1.x - v2.x,
    v1.y - v2.y,
    v1.z - v2.z,
  };

  return difference;
}

float4x4 operator*(const float4x4& m1, const float4x4& m2)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  const float4 col_12 = { m2[0][0], m2[1][0], m2[2][0], m2[3][0] };
  const float4 col_22 = { m2[0][1], m2[1][1], m2[2][1], m2[3][1] };
  const float4 col_32 = { m2[0][2], m2[1][2], m2[2][2], m2[3][2] };
  const float4 col_42 = { m2[0][3], m2[1][3], m2[2][3], m2[3][3] };

  float4x4 mul =
  {{
    {{ dot(row_11, col_12), dot(row_11, col_22), dot(row_11, col_32), dot(row_11, col_42) }},
    {{ dot(row_21, col_12), dot(row_21, col_22), dot(row_21, col_32), dot(row_21, col_42) }},
    {{ dot(row_31, col_12), dot(row_31, col_22), dot(row_31, col_32), dot(row_31, col_42) }},
    {{ dot(row_41, col_12), dot(row_41, col_22), dot(row_41, col_32), dot(row_41, col_42) }},
  }};

  return mul;
}

float4 operator*(const float4x4& m1, const float4& v1)
{
  const float4 row_11 = { m1[0][0], m1[0][1], m1[0][2], m1[0][3] };
  const float4 row_21 = { m1[1][0], m1[1][1], m1[1][2], m1[1][3] };
  const float4 row_31 = { m1[2][0], m1[2][1], m1[2][2], m1[2][3] };
  const float4 row_41 = { m1[3][0], m1[3][1], m1[3][2], m1[3][3] };

  float4 mul =
  {
    dot(row_11, v1),
    dot(row_21, v1),
    dot(row_31, v1),
    dot(row_41, v1),
  };

  return mul;
}

float2& operator-=(float2& v1, const float2& v2)
{
  v1.x = v1.x - v2.x;
  v1.y = v1.y - v2.y;

  return v1;
}

float2 operator+(const float2& v1, const float s1) { return {v1.x + s1, v1.y + s1}; }
float2 operator+(const float2& v1, const float2& v2) { return {v1.x + v2.x, v1.y + v2.y}; }

class MatrixDemo : public tDX::PixelGameEngine
{
public:
  MatrixDemo()
  {
    sAppName = "3D matrix demo";
  }

  bool OnUserCreate() override
  {
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);

    // Keyboard control
    const float coeficient = 2.0f * fElapsedTime;

    if (GetKey(tDX::D).bHeld) { m_cubeTranslationX += coeficient; }
    if (GetKey(tDX::A).bHeld) { m_cubeTranslationX -= coeficient; }
    if (GetKey(tDX::W).bHeld) { m_cubeTranslationZ -= coeficient; }
    if (GetKey(tDX::S).bHeld) { m_cubeTranslationZ += coeficient; }
    if (GetKey(tDX::E).bHeld) { m_yaw += coeficient * 30; }
    if (GetKey(tDX::Q).bHeld) { m_yaw -= coeficient * 30; }

    m_cubeTranslationZ = max(m_cubeTranslationZ, -5.0f);
    m_cubeTranslationZ = min(m_cubeTranslationZ, -2.0f);

    m_cubeTranslationX = max(m_cubeTranslationX, -5.0f);
    m_cubeTranslationX = min(m_cubeTranslationX, 4.5f);

    m_yaw = fmod(m_yaw, 360.0f);

    // Grid
    for (uint8_t row = 0; row < m_gridRows; row++)
      DrawLine(0, row * m_cellSize, m_windowWidth - 1, row * m_cellSize, tDX::VERY_DARK_GREY);

    for (uint8_t col = 0; col < m_gridCols; col++)
      DrawLine(col * m_cellSize, 0, col * m_cellSize, m_windowHeight - 1, tDX::VERY_DARK_GREY);

    // Axes
    DrawLine(0, m_originY, m_windowWidth - 1, m_originY, tDX::DARK_YELLOW);
    DrawLine(m_originX, 0, m_originX, m_windowHeight - 1, tDX::DARK_YELLOW);

    // Camera
    DrawRect(m_originX - 4, m_originY - 5 + 10, 8, 10, tDX::BLUE);
    DrawRect(m_originX - 2, m_originY - 10 + 10, 4, 4, tDX::BLUE);

    // Draw frustum
    float fovx = 2 * atan(tan(toRad(45.0f * 0.5)) * m_aspectRatio);
    float length = (tan(fovx / 2.0f) * m_windowHeight);

    DrawLineClipped(m_originX, m_originY, m_originX - length, m_originY - m_windowHeight, { 0, 0 }, { m_windowWidth - 1, m_windowHeight - 1 }, tDX::BLUE);
    DrawLineClipped(m_originX, m_originY, m_originX + length, m_originY - m_windowHeight, { 0, 0 }, { m_windowWidth - 1, m_windowHeight - 1 }, tDX::BLUE);

    // 2D square
    float2 leftUp = {m_originX - m_cellSize + (m_cubeTranslationX * m_cellSize * 2), m_originY - m_cellSize + (m_cubeTranslationZ * m_cellSize * 2) };
    float size = m_cellSize * 2.0f;

    array<float2, 4> m_rectangle =
    {{
      {leftUp.x       , leftUp.y       },
      {leftUp.x + size, leftUp.y       },
      {leftUp.x + size, leftUp.y + size},
      {leftUp.x       , leftUp.y + size},
    }};

    float2 centerVertex = leftUp + m_cellSize;

    for (auto& vertex : m_rectangle)
    {
      vertex -= centerVertex;

      float2 rotatedVertex;
      rotatedVertex.x = vertex.x * cos(toRad(m_yaw)) - vertex.y * sin(toRad(m_yaw));
      rotatedVertex.y = vertex.x * sin(toRad(m_yaw)) + vertex.y * cos(toRad(m_yaw));

      vertex = rotatedVertex + centerVertex;
    }

    DrawLine(lround(m_rectangle[0].x), lround(m_rectangle[0].y), lround(m_rectangle[1].x), lround(m_rectangle[1].y), tDX::RED);
    DrawLine(lround(m_rectangle[1].x), lround(m_rectangle[1].y), lround(m_rectangle[2].x), lround(m_rectangle[2].y), tDX::RED);
    DrawLine(lround(m_rectangle[2].x), lround(m_rectangle[2].y), lround(m_rectangle[3].x), lround(m_rectangle[3].y), tDX::RED);
    DrawLine(lround(m_rectangle[3].x), lround(m_rectangle[3].y), lround(m_rectangle[0].x), lround(m_rectangle[0].y), tDX::RED);

    DrawCircle(lround(m_rectangle[0].x), lround(m_rectangle[0].y), 2, tDX::YELLOW);

    // World matrix
    m_translationMatrix =
    {{
      {{ 1, 0, 0, m_cubeTranslationX }},
      {{ 0, 1, 0, 0                  }},
      {{ 0, 0, 1, m_cubeTranslationZ }},
      {{ 0, 0, 0, 1                  }},
    }};

    m_rotationMatrix =
    {{
      {{ cos(toRad(m_yaw))     , 0, -sin(toRad(m_yaw)), 0 }},
      {{ 0                     , 1, 0                      , 0 }},
      {{ sin(toRad(m_yaw)), 0, cos(toRad(m_yaw)) , 0 }},
      {{ 0                     , 0, 0                      , 1 }},
    }};

    m_modelMatrix = m_translationMatrix * m_rotationMatrix;

    // View matrix
    float3 zaxis = normalize(m_eye - m_target);
    float3 xaxis = normalize(cross(m_up, zaxis));
    float3 yaxis = cross(zaxis, xaxis);

    m_viewMatrix =
    { {
      {{ xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, m_eye) }},
      {{ yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, m_eye) }},
      {{ zaxis.x, zaxis.y, zaxis.z, -dot(zaxis, m_eye) }},
      {{ 0, 0, 0, 1 }}
    } };

    // Projection
    float fovY = 45.0f;
    float n = 0.1f;
    float f = 5.0f;
    float tan_fovY = tan(toRad(fovY/2.0f));

    float yScale = 1.0f / tan(toRad(fovY / 2.0f));
    float xScale = yScale / m_aspectRatio;

    m_projectionMatrix =
    {{
      {{ xScale, 0     , 0      , 0               }},
      {{ 0     , yScale, 0      , 0               }},
      {{ 0     , 0     , f/(n-f), n * f / (n - f) }},
      {{ 0     , 0     , -1     , 0               }}
    }};

    m_mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;

    // 3D view
    const int32_t originX3D = m_windowWidth / 2;
    const int32_t originY3D = m_windowHeight + m_windowHeight / 2;

    DrawLine(0, originY3D, m_windowWidth - 1, originY3D, tDX::DARK_YELLOW);
    DrawLine(originX3D, m_windowHeight, originX3D, m_windowHeight + m_windowHeight - 1, tDX::DARK_YELLOW);

    // Cube
    array<float4, 8> transformedCube = m_cube;

    for (auto& vertex : transformedCube)
    {
      vertex = m_mvpMatrix * vertex;

      vertex.x = vertex.x / vertex.w;
      vertex.y = vertex.y / vertex.w;
      vertex.z = vertex.z / vertex.w;
      vertex.w = 1.0f / vertex.w;

      // Viewport
      vertex.x = (vertex.x + 1.0f) * (m_windowWidth - 1) * 0.5f + 0.0f; // plus X viewport origin
      vertex.y = (1.0f - vertex.y) * (m_windowHeight - 1) * 0.5f + m_windowHeight; // plus Y viewport origin
    }

    tDX::vi2d clipWinPos = { 0, m_windowHeight };
    tDX::vi2d clipWinSize = { m_windowWidth - 1, m_windowHeight - 1 };

    DrawLineClipped(transformedCube[0].x, transformedCube[0].y, transformedCube[1].x, transformedCube[1].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[1].x, transformedCube[1].y, transformedCube[2].x, transformedCube[2].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[2].x, transformedCube[2].y, transformedCube[3].x, transformedCube[3].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[3].x, transformedCube[3].y, transformedCube[0].x, transformedCube[0].y, clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(transformedCube[4].x, transformedCube[4].y, transformedCube[5].x, transformedCube[5].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[5].x, transformedCube[5].y, transformedCube[6].x, transformedCube[6].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[6].x, transformedCube[6].y, transformedCube[7].x, transformedCube[7].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[7].x, transformedCube[7].y, transformedCube[4].x, transformedCube[4].y, clipWinPos, clipWinSize, tDX::WHITE);

    DrawLineClipped(transformedCube[0].x, transformedCube[0].y, transformedCube[4].x, transformedCube[4].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[1].x, transformedCube[1].y, transformedCube[5].x, transformedCube[5].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[2].x, transformedCube[2].y, transformedCube[6].x, transformedCube[6].y, clipWinPos, clipWinSize, tDX::WHITE);
    DrawLineClipped(transformedCube[3].x, transformedCube[3].y, transformedCube[7].x, transformedCube[7].y, clipWinPos, clipWinSize, tDX::WHITE);

    if (transformedCube[0].x > 0 && transformedCube[0].x < m_windowWidth && transformedCube[0].y > m_windowHeight && transformedCube[0].y < g::screenHeight)
      DrawCircle(lround(transformedCube[0].x), lround(transformedCube[0].y), 2, tDX::YELLOW);

    // Windows borders
    DrawRect(0, 0, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);
    DrawRect(0, m_windowHeight, m_windowWidth - 1, m_windowHeight - 1, tDX::WHITE);

    // Print matrices
    float4 worldVertex = m_modelMatrix * m_cube[0];
    float4 viewVertex = m_viewMatrix * worldVertex;
    float4 projVertex = m_projectionMatrix * viewVertex;

    stringstream modelMatrixToPrint;

    modelMatrixToPrint << fixed << setprecision(2) <<
      setw(6) << m_modelMatrix[0][0] << setw(6) << m_modelMatrix[0][1] << setw(6) << m_modelMatrix[0][2] << setw(6) << m_modelMatrix[0][3] << "    |" << setw(5) << worldVertex.x << "|" << '\n' <<
      setw(6) << m_modelMatrix[1][0] << setw(6) << m_modelMatrix[1][1] << setw(6) << m_modelMatrix[1][2] << setw(6) << m_modelMatrix[1][3] << "    |" << setw(5) << worldVertex.y << "|" << '\n' <<
      setw(6) << m_modelMatrix[2][0] << setw(6) << m_modelMatrix[2][1] << setw(6) << m_modelMatrix[2][2] << setw(6) << m_modelMatrix[2][3] << "    |" << setw(5) << worldVertex.z << "|" << '\n' <<
      setw(6) << m_modelMatrix[3][0] << setw(6) << m_modelMatrix[3][1] << setw(6) << m_modelMatrix[3][2] << setw(6) << m_modelMatrix[3][3] << "    |" << setw(5) << worldVertex.w << "|" << '\n';

    DrawString(310, 10, "Model to world");
    DrawString(300, 25, modelMatrixToPrint.str());

    stringstream lookAtToPrint;

    lookAtToPrint << fixed << setprecision(2) <<
      "  Eye    " << setw(6) << m_eye.x << setw(6) << m_eye.y << setw(6) << m_eye.z << '\n' <<
      "  Target " << setw(6) << m_target.x << setw(6) << m_target.y << setw(6) << m_target.z << '\n' <<
      "  Up     " << setw(6) << m_up.x << setw(6) << m_up.y << setw(6) << m_up.z << '\n';

    DrawString(310, 70, "LookAt input data", tDX::GREY);
    DrawString(300, 85, lookAtToPrint.str(), tDX::GREY);

    stringstream viewMatrixToPrint;

    viewMatrixToPrint << fixed << setprecision(2) <<
      setw(6) << m_viewMatrix[0][0] << setw(6) << m_viewMatrix[0][1] << setw(6) << m_viewMatrix[0][2] << setw(6) << m_viewMatrix[0][3] << "    |" << setw(5) << viewVertex.x << "|" << '\n' <<
      setw(6) << m_viewMatrix[1][0] << setw(6) << m_viewMatrix[1][1] << setw(6) << m_viewMatrix[1][2] << setw(6) << m_viewMatrix[1][3] << "    |" << setw(5) << viewVertex.y << "|" << '\n' <<
      setw(6) << m_viewMatrix[2][0] << setw(6) << m_viewMatrix[2][1] << setw(6) << m_viewMatrix[2][2] << setw(6) << m_viewMatrix[2][3] << "    |" << setw(5) << viewVertex.z << "|" << '\n' <<
      setw(6) << m_viewMatrix[3][0] << setw(6) << m_viewMatrix[3][1] << setw(6) << m_viewMatrix[3][2] << setw(6) << m_viewMatrix[3][3] << "    |" << setw(5) << viewVertex.w << "|" << '\n';

    DrawString(310, 130, "world to View");
    DrawString(300, 145, viewMatrixToPrint.str());

    stringstream projectionMatrixToPrint;

    projectionMatrixToPrint << fixed << setprecision(2) <<
      setw(6) << m_projectionMatrix[0][0] << setw(6) << m_projectionMatrix[0][1] << setw(6) << m_projectionMatrix[0][2] << setw(6) << m_projectionMatrix[0][3] << "    |" << setw(5) << projVertex.x << "|" << '\n' <<
      setw(6) << m_projectionMatrix[1][0] << setw(6) << m_projectionMatrix[1][1] << setw(6) << m_projectionMatrix[1][2] << setw(6) << m_projectionMatrix[1][3] << "    |" << setw(5) << projVertex.y << "|" << '\n' <<
      setw(6) << m_projectionMatrix[2][0] << setw(6) << m_projectionMatrix[2][1] << setw(6) << m_projectionMatrix[2][2] << setw(6) << m_projectionMatrix[2][3] << "    |" << setw(5) << projVertex.z << "|" << '\n' <<
      setw(6) << m_projectionMatrix[3][0] << setw(6) << m_projectionMatrix[3][1] << setw(6) << m_projectionMatrix[3][2] << setw(6) << m_projectionMatrix[3][3] << "    |" << setw(5) << projVertex.w << "|" << '\n';

    DrawString(310, 190, "view to Projection");
    DrawString(300, 205, projectionMatrixToPrint.str());

    stringstream mvpMatrixToPrint;

    mvpMatrixToPrint << fixed << setprecision(2) <<
      setw(6) << m_mvpMatrix[0][0] << setw(6) << m_mvpMatrix[0][1] << setw(6) << m_mvpMatrix[0][2] << setw(6) << m_mvpMatrix[0][3] << '\n' <<
      setw(6) << m_mvpMatrix[1][0] << setw(6) << m_mvpMatrix[1][1] << setw(6) << m_mvpMatrix[1][2] << setw(6) << m_mvpMatrix[1][3] << '\n' <<
      setw(6) << m_mvpMatrix[2][0] << setw(6) << m_mvpMatrix[2][1] << setw(6) << m_mvpMatrix[2][2] << setw(6) << m_mvpMatrix[2][3] << '\n' <<
      setw(6) << m_mvpMatrix[3][0] << setw(6) << m_mvpMatrix[3][1] << setw(6) << m_mvpMatrix[3][2] << setw(6) << m_mvpMatrix[3][3] << '\n';

    DrawString(310, 250, "MVP matrix", tDX::GREY);
    DrawString(300, 265, mvpMatrixToPrint.str(), tDX::GREY);

    stringstream cubePointPrint;

    cubePointPrint << fixed << setprecision(1) <<
      setw(7) << transformedCube[0].x << setw(7) << transformedCube[0].y << setw(5) << transformedCube[0].z << setw(5) << transformedCube[0].w << '\n';

    DrawString(310, 310, "Cube vertex in screen space");
    DrawString(300, 325, cubePointPrint.str());

    return true;
  }

private:
  // Constants to specify UI
  constexpr static int32_t m_windowWidth = g::screenWidth / 2;
  constexpr static int32_t m_windowHeight = g::screenHeight / 2;
  constexpr static int32_t m_originX = 156;
  constexpr static int32_t m_originY = 156;
  constexpr static int32_t m_cellSize = m_windowHeight / 14;
  constexpr static uint8_t m_gridRows = m_windowHeight / m_cellSize + 1;
  constexpr static uint8_t m_gridCols = m_windowWidth / m_cellSize + 1;

  constexpr static float m_aspectRatio = (float)m_windowWidth / (float)m_windowHeight;

  // Model
  constexpr static array<float4, 8> m_cube =
  {{
    {-0.5, -0.5, -0.5, 1.0 },
    { 0.5, -0.5, -0.5, 1.0 },
    { 0.5,  0.5, -0.5, 1.0 },
    {-0.5,  0.5, -0.5, 1.0 },
    {-0.5, -0.5,  0.5, 1.0 },
    { 0.5, -0.5,  0.5, 1.0 },
    { 0.5,  0.5,  0.5, 1.0 },
    {-0.5,  0.5,  0.5, 1.0 }
  }};

  // Default matrix
  constexpr static float4x4 m_identityMatrix =
  {{
    {{ 1, 0, 0, 0 }},
    {{ 0, 1, 0, 0 }},
    {{ 0, 0, 1, 0 }},
    {{ 0, 0, 0, 1 }}
  }};

  // Cube transformations
  float m_cubeTranslationX = 0.0f;
  float m_cubeTranslationZ = -2.0f;
  float m_yaw = 0;

  // Matrices to describe a scene
  float4x4 m_translationMatrix;
  float4x4 m_rotationMatrix;
  float4x4 m_modelMatrix;

  float4x4 m_viewMatrix;
  float4x4 m_projectionMatrix;

  float4x4 m_mvpMatrix;

  // Look at
  float3 m_eye = { 0, 0, 0 };
  float3 m_target = { 0, 0, -1 };
  float3 m_up = { 0, 1, 0 };
};

int main()
{
  MatrixDemo demo;
  if (demo.Construct(g::screenWidth, g::screenHeight, 2, 2))
    demo.Start();

  return 0;
}

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define NOMINMAX      // Must come before Windows.h
#include <Windows.h>  // For console font manipulation

// Simple 3D vector class for mathematical operations
struct Vec3
{
  float x, y, z;

  Vec3() : x(0), y(0), z(0) {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  // Vector operations
  Vec3 operator+(const Vec3& v) const
  {
    return Vec3(x + v.x, y + v.y, z + v.z);
  }

  Vec3 operator-(const Vec3& v) const
  {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }

  Vec3 operator*(float s) const
  {
    return Vec3(x * s, y * s, z * s);
  }

  // Returns normalized version of the vector
  Vec3 normalize() const
  {
    float mag = sqrt(x * x + y * y + z * z);
    return Vec3(x / mag, y / mag, z / mag);
  }

  // Dot product calculation
  float dot(const Vec3& v) const
  {
    return x * v.x + y * v.y + z * v.z;
  }
};

// Represents a ray with origin and direction
struct Ray
{
  Vec3 origin;
  Vec3 direction;

  Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d)
  {
  }
};

// Sphere object with position, radius, and color
struct Sphere
{
  Vec3 center;
  float radius;
  Vec3 color;

  Sphere(const Vec3& c, float r, const Vec3& col)
    : center(c), radius(r), color(col)
  {
  }

  // Ray-sphere intersection test using quadratic formula
  bool intersect(const Ray& ray, float& t) const
  {
    Vec3 offset = ray.origin - center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * offset.dot(ray.direction);
    float c = offset.dot(offset) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float sqrt_d = sqrt(discriminant);
    float t0 = (-b - sqrt_d) / (2 * a);
    float t1 = (-b + sqrt_d) / (2 * a);

    t = (t0 < t1 && t0 >= 0) ? t0 : t1;
    return t >= 0;
  }
};

// Traces a ray through the scene and calculates color
Vec3 trace(const Ray& ray, const std::vector<Sphere>& spheres, const Vec3& light)
{
  float min_t = INFINITY;
  const Sphere* closest = nullptr;

  // Find closest intersecting sphere
  for (const auto& sphere : spheres)
  {
    float t;
    if (sphere.intersect(ray, t) && t < min_t)
    {
      min_t = t;
      closest = &sphere;
    }
  }

  // Return background color if no intersection
  if (!closest)
  {
    return Vec3(0.2f, 0.7f, 0.8f);  // Sky blue background
  }

  // Calculate surface properties at intersection point
  Vec3 hit_point = ray.origin + ray.direction * min_t;
  Vec3 normal = (hit_point - closest->center).normalize();
  Vec3 light_dir = (light - hit_point).normalize();

  // Diffuse lighting calculation
  float diffuse = std::max(0.0f, normal.dot(light_dir));

  // Specular highlight calculation
  Vec3 view_dir = (ray.origin - hit_point).normalize();
  Vec3 reflect_dir = light_dir - normal * 2.0f * normal.dot(light_dir);
  float specular = pow(std::max(0.0f, view_dir.dot(reflect_dir)), 32);

  // Combine material color with lighting
  return closest->color * (diffuse + 0.3f) + Vec3(1, 1, 1) * specular * 0.5f;
}

// Configures console font for square ASCII output
void setSquareFont()
{
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_FONT_INFOEX fontInfo = { sizeof(fontInfo) };
  fontInfo.dwFontSize.X = 8;
  fontInfo.dwFontSize.Y = 8;
  wcscpy_s(fontInfo.FaceName, L"Terminal");
  SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
}

int main()
{
  setSquareFont();

  // Scene configuration
  const int width = 80;
  const int height = 40;
  const std::string gradient = " .:-=+*#%@";  // ASCII brightness gradient

  Vec3 camera(0, 0, 3);    // Camera position
  Vec3 light(-5, 5, 5);    // Light position
  std::vector<Sphere> spheres = {
      Sphere(Vec3(0, 0, 0), 1, Vec3(1, 0.2f, 0.2f)),     // Red sphere
      Sphere(Vec3(2, 0.5f, -1), 0.5f, Vec3(0.2f, 1, 0.2f)),  // Green sphere
      Sphere(Vec3(-1, 0, 0), 0.8f, Vec3(0.2f, 0.2f, 1))  // Blue sphere
  };

  // Render loop
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      // Convert screen coordinates to normalized device coordinates
      float u = (x - width / 2.0f) / width * 2.0f;
      float v = (height / 2.0f - y) / height * 2.0f * 0.5f;

      // Create ray through current pixel
      Ray ray(camera, Vec3(u, v, -1).normalize());
      Vec3 color = trace(ray, spheres, light);

      // Convert color to ASCII character
      float luminance = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
      luminance = std::clamp(luminance, 0.0f, 1.0f);
      int char_index = static_cast<int>(luminance * (gradient.length() - 1));

      std::cout << gradient[char_index];
    }

    std::cout << "\n";
  }

  return 0;
}
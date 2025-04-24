#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>

// Constants for display dimensions (ASCII art "screen" size)
const int WIDTH = 80;
const int HEIGHT = 40;

// Constants for 3D rendering
const float CUBE_SIZE = 12.0f;       // Controls the size of the cube
const float CAMERA_DISTANCE = 3.0f;  // Distance from camera to cube (for perspective)
const float FOV = 90.0f;             // Field of view in degrees

// ASCII characters used for shading based on depth (from darkest to lightest)
const char SHADES[] = ".:-=+*#%@";

// Structure to represent a 3D point
struct Point3D
{
  float x, y, z;
};

// Structure to represent a 2D point with depth information for z-buffering/shading
struct Point2D
{
  int x, y;
  float depth;
};

// Vertices of a unit cube centered at the origin
std::vector<Point3D> cube_vertices =
{
    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
    {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1}
};

// Cube edges as pairs of vertex indices
std::vector<std::pair<int, int>> cube_edges =
{
    {0,1}, {1,2}, {2,3}, {3,0}, // bottom face
    {4,5}, {5,6}, {6,7}, {7,4}, // top face
    {0,4}, {1,5}, {2,6}, {3,7}  // vertical edges
};

// Projects a 3D point onto the 2D "screen" using perspective projection
Point2D project(const Point3D& p)
{
  // Calculate aspect ratio for proper scaling
  float aspect = static_cast<float>(WIDTH) / HEIGHT;

  // Calculate scale factor based on field of view (convert FOV to radians)
  float fovScale = 1.0f / tan(FOV * 0.5f * 3.14 / 180);

  // Apply perspective by adding camera distance to z
  float z = p.z + CAMERA_DISTANCE;

  // Perspective divide and scaling
  float scale = fovScale / z * CUBE_SIZE;

  // Convert 3D coordinates to 2D screen coordinates
  return
  {
      static_cast<int>(WIDTH / 2 + p.x * scale * aspect), // X: center + scaled position
      static_cast<int>(HEIGHT / 2 - p.y * scale),         // Y: center - scaled position (Y axis inverted)
      z                                                   // Store depth for shading
  };
}

// Rotates a point around the X axis by angle a (in radians)
void rotateX(Point3D& p, float a)
{
  float y = p.y * cos(a) - p.z * sin(a);
  float z = p.y * sin(a) + p.z * cos(a);
  p.y = y;
  p.z = z;
}

// Rotates a point around the Y axis by angle a (in radians)
void rotateY(Point3D& p, float a)
{
  float x = p.x * cos(a) + p.z * sin(a);
  float z = -p.x * sin(a) + p.z * cos(a);
  p.x = x;
  p.z = z;
}

// Draws a line between two 2D points using Bresenham's algorithm, with depth-based shading
void drawLine(std::vector<std::vector<char>>& screen,
  const Point2D& p1, const Point2D& p2)
{
  // Calculate average depth for the line to determine shading
  float avgDepth = (p1.depth + p2.depth) / 2;

  // Select shading character based on depth
  int shadeIdx = static_cast<int>((avgDepth - CAMERA_DISTANCE) * 2) % sizeof(SHADES);
  shadeIdx = std::clamp(shadeIdx, 0, static_cast<int>(sizeof(SHADES) - 2));
  char c = SHADES[shadeIdx];

  // Initialize Bresenham's line algorithm variables
  int dx = abs(p2.x - p1.x), dy = -abs(p2.y - p1.y);
  int sx = p1.x < p2.x ? 1 : -1, sy = p1.y < p2.y ? 1 : -1;
  int err = dx + dy, e2;
  int x = p1.x, y = p1.y;

  // Bresenham's line drawing loop
  while (true)
  {
    // Draw pixel if within screen bounds
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    {
      // Only draw if cell is empty or new shade is darker (higher index)
      if (screen[y][x] == ' ' || SHADES[shadeIdx] > screen[y][x])
      {
        screen[y][x] = c;
      }
    }

    // Check if we've reached the end point
    if (x == p2.x && y == p2.y) break;

    // Update error and coordinates (Bresenham's algorithm)
    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x += sx;
    }
    if (e2 <= dx)
    {
      err += dx;
      y += sy;
    }
  }
}

int main()
{
  // Disable IO synchronization for faster output
  std::ios_base::sync_with_stdio(false);

  // Hide the terminal cursor for a cleaner animation
  std::cout << "\033[?25l";

  // Initialize rotation angles
  float angleX = 0, angleY = 0;

  // Pre-allocate buffer for screen output to minimize allocations
  std::string buffer;
  buffer.reserve((WIDTH + 1) * HEIGHT + 10);

  // Main animation loop
  while (true)
  {
    // Clear buffer and reset cursor position in terminal
    buffer.clear();
    buffer += "\033[H"; // ANSI escape: move cursor to home position

    // Initialize the screen as a 2D array filled with spaces
    std::vector<std::vector<char>> screen(HEIGHT, std::vector<char>(WIDTH, ' '));

    // Copy cube vertices for rotation
    std::vector<Point3D> rotated = cube_vertices;

    // Rotate all vertices around X and Y axes
    for (auto& v : rotated)
    {
      rotateX(v, angleX);
      rotateY(v, angleY);
    }

    // Project all rotated 3D points to 2D screen coordinates
    std::vector<Point2D> projected;
    for (const auto& v : rotated)
    {
      projected.push_back(project(v));
    }

    // Draw all cube edges onto the screen buffer
    for (const auto& e : cube_edges)
    {
      drawLine(screen, projected[e.first], projected[e.second]);
    }

    // Convert the screen buffer to a single string (for fast output)
    for (const auto& row : screen)
    {
      buffer.append(row.begin(), row.end());
      buffer += '\n';
    }

    // Output the frame to the terminal
    std::cout << buffer << std::flush;

    // Increment rotation angles for next frame (animation)
    angleX += 0.03f;
    angleY += 0.02f;

    // Sleep briefly to control animation speed (~200 FPS)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // Restore cursor visibility before exit
  std::cout << "\033[?25h";

  return 0;
}

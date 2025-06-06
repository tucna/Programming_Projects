#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <set>
#include <fstream>
#include <sstream>

#define NOMINMAX      // Must come before Windows.h
#include <Windows.h>  // For console font manipulation

// Constants for display dimensions (ASCII art "screen" size)
const int WIDTH = 170;
const int HEIGHT = 170;

// Constants for 3D rendering
const float MODEL_SIZE = 150.0f;       // Controls the size of the cube
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

// Global variables for model data
std::vector<Point3D> model_vertices;
std::vector<std::pair<int, int>> model_edges;

// Default cube vertices
std::vector<Point3D> cube_vertices =
{
    {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
    {-1,-1,1}, {1,-1,1}, {1,1,1}, {-1,1,1}
};

// Default cube edges as pairs of vertex indices
std::vector<std::pair<int, int>> cube_edges =
{
    {0,1}, {1,2}, {2,3}, {3,0}, // bottom face
    {4,5}, {5,6}, {6,7}, {7,4}, // top face
    {0,4}, {1,5}, {2,6}, {3,7}  // vertical edges
};

// Function to load OBJ file and extract vertices and edges
bool loadOBJ(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Error: Could not open OBJ file: " << filename << std::endl;
    return false;
  }

  model_vertices.clear();
  model_edges.clear();

  std::string line;
  std::vector<std::vector<int>> faces; // Store face data to generate edges

  while (std::getline(file, line))
  {
    if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments

    std::istringstream iss(line);
    std::string type;
    iss >> type;

    if (type == "v") // Vertex
    {
      float x, y, z;
      if (iss >> x >> y >> z)
      {
        model_vertices.push_back({ x, y, z });
      }
    }
    else if (type == "f") // Face
    {
      std::vector<int> face_vertices;
      std::string vertex_data;

      while (iss >> vertex_data)
      {
        // Handle different OBJ face formats (v, v/vt, v/vt/vn, v//vn)
        std::size_t slash_pos = vertex_data.find('/');
        std::string vertex_index_str = vertex_data.substr(0, slash_pos);

        int vertex_index = std::stoi(vertex_index_str);

        // OBJ indices are 1-based, convert to 0-based
        if (vertex_index > 0)
        {
          face_vertices.push_back(vertex_index - 1);
        }
        else if (vertex_index < 0)
        {
          // Negative indices count from end
          face_vertices.push_back(model_vertices.size() + vertex_index);
        }
      }

      if (face_vertices.size() >= 3)
      {
        faces.push_back(face_vertices);
      }
    }
    else if (type == "l") // Line (some OBJ files have explicit line definitions)
    {
      std::vector<int> line_vertices;
      std::string vertex_data;

      while (iss >> vertex_data)
      {
        int vertex_index = std::stoi(vertex_data);
        if (vertex_index > 0)
        {
          line_vertices.push_back(vertex_index - 1);
        }
        else if (vertex_index < 0)
        {
          line_vertices.push_back(model_vertices.size() + vertex_index);
        }
      }

      // Create edges from consecutive vertices in the line
      for (size_t i = 0; i < line_vertices.size() - 1; ++i)
      {
        model_edges.push_back({ line_vertices[i], line_vertices[i + 1] });
      }
    }
  }

  file.close();

  if (model_vertices.empty())
  {
    std::cerr << "Error: No vertices found in OBJ file" << std::endl;
    return false;
  }

  // Generate edges from faces if no explicit lines were found
  if (model_edges.empty())
  {
    std::set<std::pair<int, int>> edge_set; // Use set to avoid duplicate edges

    for (const auto& face : faces)
    {
      // Create edges for each face (wireframe representation)
      for (size_t i = 0; i < face.size(); ++i)
      {
        int v1 = face[i];
        int v2 = face[(i + 1) % face.size()]; // Next vertex (wrapping around)

        // Ensure consistent edge ordering (smaller index first)
        if (v1 > v2) std::swap(v1, v2);
        edge_set.insert({ v1, v2 });
      }
    }

    // Convert set to vector
    model_edges.assign(edge_set.begin(), edge_set.end());
  }

  // Normalize model to fit within reasonable bounds
  if (!model_vertices.empty())
  {
    float min_x = model_vertices[0].x, max_x = model_vertices[0].x;
    float min_y = model_vertices[0].y, max_y = model_vertices[0].y;
    float min_z = model_vertices[0].z, max_z = model_vertices[0].z;

    // Find bounding box
    for (const auto& v : model_vertices)
    {
      min_x = std::min(min_x, v.x); max_x = std::max(max_x, v.x);
      min_y = std::min(min_y, v.y); max_y = std::max(max_y, v.y);
      min_z = std::min(min_z, v.z); max_z = std::max(max_z, v.z);
    }

    // Calculate center and scale
    float center_x = (min_x + max_x) / 2;
    float center_y = (min_y + max_y) / 2;
    float center_z = (min_z + max_z) / 2;

    float size_x = max_x - min_x;
    float size_y = max_y - min_y;
    float size_z = max_z - min_z;
    float max_size = std::max({ size_x, size_y, size_z });

    float scale = (max_size > 0) ? 2.0f / max_size : 1.0f; // Scale to fit in [-1, 1] range

    // Apply centering and scaling
    for (auto& v : model_vertices)
    {
      v.x = (v.x - center_x) * scale;
      v.y = (v.y - center_y) * scale;
      v.z = (v.z - center_z) * scale;
    }
  }

  std::cout << "Loaded OBJ file: " << model_vertices.size() << " vertices, "
    << model_edges.size() << " edges" << std::endl;

  return true;
}

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
  float scale = fovScale / z * MODEL_SIZE;

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

int main(int argc, char* argv[])
{
  setSquareFont();

  // Disable IO synchronization for faster output
  std::ios_base::sync_with_stdio(false);

  // Set default model data (cube)
  model_vertices = cube_vertices;
  model_edges = cube_edges;

  // Check if OBJ file was provided as command line argument
  if (argc > 1)
  {
    std::string objFile = argv[1];
    std::cout << "Attempting to load OBJ file: " << objFile << std::endl;

    if (!loadOBJ(objFile))
    {
      std::cout << "Failed to load OBJ file, using default cube" << std::endl;
      // Keep default cube data
    }
  }
  else
  {
    std::cout << "No OBJ file specified. Usage: " << argv[0] << " [filename.obj]" << std::endl;
    std::cout << "Using default cube model" << std::endl;
  }

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

    // Copy model vertices for rotation
    std::vector<Point3D> rotated = model_vertices;

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

    // Draw all model edges onto the screen buffer
    for (const auto& e : model_edges)
    {
      if (e.first < projected.size() && e.second < projected.size())
      {
        drawLine(screen, projected[e.first], projected[e.second]);
      }
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

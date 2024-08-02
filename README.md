# c++ raylib hair physics

This code provides a dynamic simulation of hair physics using spring-mass models and gravity, visualized in real-time with user interaction capabilities for adjusting simulation parameters and visual effects.




Segments have physical properties like mass and spring force. They interact with each other through spring forces (F = K * (d - L)) and gravity (F = m * g), Based on computed velocities (v = v + (F / m) * dt), the positions of the segments are updated (p = p + v * dt), After updating positions, a correction is applied to ensure segments maintain the correct length. To improve efficiency, the hair physics calculations are performed in parallel across multiple threads

raylib and c++

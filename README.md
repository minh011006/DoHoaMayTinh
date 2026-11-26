# DoHoaMayTinh

## Lưới bình hoa tròn xoay

Module C++17 `vase_mesh` nhận các điểm đã lấy mẫu từ Bézier theo thứ tự đáy đến miệng. Module quay đường biên quanh trục Y và dùng một đỉnh chung cho mỗi đầu có bán kính bằng 0. Đầu có bán kính dương được giữ mở. Module có thể biên dịch độc lập với OpenGL/GLUT.

```cpp
#include "vase_mesh.hpp"

const std::vector<vase::ProfilePoint> profile = {
    {0.0, 0.0}, {1.0, 0.0}, {1.3, 1.2}, {0.8, 2.4}
};
const vase::Mesh mesh = vase::generateVaseMesh(profile, 48);
```

App có thể liên kết target CMake `vase_mesh` hoặc biên dịch `src/vase_mesh.cpp` cùng mã nguồn của app và thêm `include/` vào đường dẫn header. `positions` và `normals` có cùng số phần tử, mỗi phần tử gồm ba giá trị `double` theo thứ tự `(x, y, z)`. Mỗi bộ ba trong `indices` tạo một tam giác, theo chiều ngược kim đồng hồ khi nhìn từ phía ngoài. Khi vẽ bằng `glDrawElements`, dùng `GL_TRIANGLES`, số phần tử `mesh.indices.size()` và kiểu chỉ số `GL_UNSIGNED_INT`.

Đường biên cần có ít nhất hai điểm, tọa độ hữu hạn, bán kính không âm và cao độ miệng bằng hoặc cao hơn cao độ đáy. Các đoạn ngang và đoạn uốn cục bộ được hỗ trợ. Bán kính 0 chỉ xuất hiện ở đầu hoặc cuối, và cần có ít nhất một điểm bán kính dương. Mỗi cặp điểm kề nhau phải khác nhau và góc đổi hướng tại mỗi điểm phải nhỏ hơn 180°. Số lát từ 3 trở lên. Hàm ném `std::invalid_argument` khi vi phạm các điều kiện này và `std::length_error` khi số đỉnh hoặc chỉ số vượt giới hạn.

## Build và chạy kiểm tra

Chạy PowerShell ở thư mục repo. Các lệnh dưới đây dùng CMake đi kèm Visual Studio Build Tools 2019 trên máy đã chạy kiểm tra.

```powershell
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
& $cmake -S . -B build -G 'Visual Studio 16 2019' -A x64
& $cmake --build build --config Release
.\build\Release\vase_mesh_check.exe
& $ctest --test-dir build -C Release --output-on-failure
```

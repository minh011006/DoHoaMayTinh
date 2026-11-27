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

## Khung chỉnh Bézier (feature/bezier-editor)

Ứng dụng `bezier_editor` dùng C++17, OpenGL và **FreeGLUT**. Khung trái gồm trục quay Y (`r=0`), bốn điểm P0–P3, đa giác điều khiển và đường Bézier bậc ba. Kéo điểm bằng chuột trái; đường cong và 101 mẫu cập nhật trong lúc kéo. P0 là đáy, P3 là miệng. Khung phải hiển thị tọa độ điều khiển và bản xem trước mỗi mẫu thứ năm.

Giới hạn tọa độ mô hình: bán kính 0.05–2.5, chiều cao 0–4; chiều cao điểm điều khiển không giảm, đáy và miệng cách nhau ít nhất 0.05. Giới hạn bán kính dương giúp phần chỉnh đường biên giữ miệng và đáy mở; nhóm có thể thêm điểm trên trục khi cần đóng đáy trong module dựng lưới. Không sắp xếp lại mẫu sau khi lấy mẫu, vì việc đó có thể thay đổi đường biên.

- `R`: khôi phục bốn điểm ban đầu.
- `S`: lưu lại CSV; `Esc`: thoát.
- CSV được ghi khi khởi động, kéo điểm và reset. Mặc định là `profile.csv` trong thư mục làm việc; có thể chọn `bezier_editor --output D:/path/profile.csv`. Lỗi ghi được hiển thị trên giao diện.
- Thay đổi kích thước cửa sổ vẫn giữ phép đổi tọa độ chuột đúng với bản vẽ.

Tích hợp phần dựng 3D:

```cpp
#include "bezier_profile.hpp"
vase::BezierProfile editor;
editor.moveControl(1, 1.4, 0.9);
auto profile = editor.sample(101); // thứ tự đáy -> miệng, đơn vị mô hình
const auto mesh = vase::generateVaseMesh(profile, 48);
```

Liên kết cả hai target `bezier_profile` và `vase_mesh` khi dùng ví dụ trên. `BezierProfile::controls` dùng để đọc vị trí; dùng `moveControl()` khi cập nhật để áp dụng ràng buộc. `sample()` trả danh sách `std::vector<ProfilePoint>` với các trường `r`, `y`. CSV có dòng tiêu đề `r,y` và 101 dòng dữ liệu, ghi đủ độ chính xác `double`.

### Build ứng dụng

Cài FreeGLUT phù hợp kiến trúc trình biên dịch, rồi dùng CMake:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/path/to/freeglut"
cmake --build build
ctest --test-dir build --output-on-failure
.\build\bezier_editor.exe
```

Nếu FreeGLUT dùng DLL, đặt DLL cạnh `bezier_editor.exe` hoặc thêm thư mục DLL vào PATH. Để chỉ build thư viện và kiểm tra mà không cần OpenGL/FreeGLUT: thêm `-DBUILD_BEZIER_EDITOR=OFF` khi cấu hình. Bộ kiểm tra mới đối chiếu công thức Bézier, đầu mút, 200 thao tác kéo có ràng buộc, tích hợp lưới và số dòng CSV.

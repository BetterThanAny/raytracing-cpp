# Ray Tracing in One Weekend — 完整 C++ 实现

基于 Peter Shirley 等人的《Ray Tracing in One Weekend》三部曲（https://raytracing.github.io/），从零用 C++17 实现离线路径追踪器，并加入多线程 + PNG 输出 + OBJ 加载等扩展。

## 目标

- 完成三本书所有章节的核心渲染器
- 所有输出图像必须可视化验证（PPM + PNG）
- 单机可跑，无外部服务依赖

## 里程碑与验收命令

### M1 Book 1 — Ray Tracing in One Weekend ✅
- [x] 1.1 PPM 图像输出 + Vec3
- [x] 1.2 Ray + 简单相机 + 背景渐变
- [x] 1.3 球体求交
- [x] 1.4 Hittable 抽象 + hit_record
- [x] 1.5 Antialiasing（每像素多采样）
- [x] 1.6 漫反射材质（Lambertian）
- [x] 1.7 金属材质（Metal + fuzz）
- [x] 1.8 介质材质（Dielectric + Schlick 近似）
- [x] 1.9 可定位相机 + 散焦模糊（Depth of Field）
- [x] 1.10 Final scene（随机球阵）
- 验收图：`output/first_light.png`, `output/book1_final_preview.png`

### M2 Book 2 — The Next Week ✅
- [x] 2.1 Motion Blur（sphere 支持 c1→c2 + ray 带 time）
- [x] 2.2 BVH 加速结构（`bvh_node`，最长轴划分）
- [x] 2.3 纹理（solid / checker / image / Perlin noise + turb）
- [x] 2.4 四边形 Quad + box 辅助
- [x] 2.5 光源 emission（`diffuse_light`）
- [x] 2.6 Cornell Box（含 translate / rotate_y 变换）
- [x] 2.7 体积 / 参与介质（`constant_medium` + `isotropic`）
- [x] 2.8 Final scene（Book 2 all-in-one）
- 验收图：`checkered_spheres.png`, `earth.png`, `perlin_spheres.png`,
  `quads.png`, `simple_light.png`, `cornell_box_preview.png`,
  `cornell_smoke_preview.png`, `book2_final_preview.png`

### M3 Book 3 — The Rest of Your Life ✅
- [x] 3.1 Monte Carlo 射线权重（scattering_pdf / pdf_value）
- [x] 3.2 PDF 抽样（sphere_pdf / cosine_pdf）
- [x] 3.3 重要性采样（hittable_pdf 对光源）
- [x] 3.4 混合 PDF（mixture_pdf 50/50）
- [x] 3.5 正交基 ONB
- [x] 3.6 scatter_record（skip_pdf 路径给镜面材质）
- 验收图：`cornell_box_mcmc.png` (600×600, 1000 spp, 228s) 对比 `cornell_box_preview.png`
  同 spp 的 brute-force，噪点显著减少

### M4 扩展 ✅
- [x] 4.1 `std::thread` 行并行（原子 work queue）
- [x] 4.2 stb_image_write PNG + PPM 双输出
- [x] 4.3 进度条（stderr、带百分比）
- [x] 4.4 OBJ mesh 加载（`obj_loader.h` 处理 v/f，支持三/四边形面）
- [x] 4.5 Möller–Trumbore 三角形求交

### M5 Bonus — 三角形网格 ✅
- [x] 5.1 Icosahedron（金 / 玻璃 / 漫反射三材质对比）— `icosahedron.png`
- [x] 5.2 Stanford Bunny（35,947 verts / 69,451 tris, BVH 加速）— `bunny_preview.png`
- ⚠️ `bunny_cornell_preview` 场景已注册但 300×300+100 spp+69k tri+重要性采样太慢，被手动终止

## 测试矩阵

| 场景 | 分辨率 | SPP | 期望渲染时间 |
|------|-------|-----|-------------|
| book1_first_image | 256x256 | 1 | <1s |
| book1_spheres | 400x225 | 10 | <5s |
| book1_final | 1200x675 | 500 | <20min 多线程 |
| book2_final | 800x800 | 1000 | <30min |
| cornell_box | 600x600 | 1000 | <20min |

## 已安装工具 / 数据

| 工具/数据 | 获取命令 | 时间 | 原因 | 清理 |
|---------|---------|------|------|------|
| external/stb_image_write.h | `curl` 单文件下载 | 2026-04-20 | PNG 输出 | `rm external/stb_image_write.h` |
| external/stb_image.h | `curl` 单文件下载 | 2026-04-20 | 图像贴图加载（earth/image textures） | `rm external/stb_image.h` |
| scenes/earthmap.jpg | 从 raytracing.github.io 仓库下载 | 2026-04-20 | `scene_earth` 地球贴图 | `rm scenes/earthmap.jpg` |
| scenes/bunny.obj | 从 alecjacobson/common-3d-test-models 下载 | 2026-04-20 | Stanford Bunny 网格 | `rm scenes/bunny.obj` |

（系统自带 clang++/cmake/make，无需额外安装。无任何 Homebrew 改动。）

## 风险 & 备注

- 单线程下 final scene 可能跑几十分钟；需尽早加入多线程
- Dielectric 数值边界用 `Schlick` 近似避免全反射穿帮
- BVH 构建要注意节点中位排序的随机选 axis，否则深度会极端

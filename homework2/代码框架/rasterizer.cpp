// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    
        // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]

        //���Ե������Ϊ(x, y)
        //���������������ֱ�Ϊ_v[0], _v[1], _v[2]

        //��˹�ʽΪ(x1, y1)X(x2, y2) = x1*y2 - y1*x2

        //��1��׼�������θ��ߵĵ�����
        Eigen::Vector2f side1;
        side1 << _v[1].x() - _v[0].x(), _v[1].y() - _v[0].y();
        Eigen::Vector2f side2;
        side2 << _v[2].x() - _v[1].x(), _v[2].y() - _v[1].y();
        Eigen::Vector2f side3;
        side3 << _v[0].x() - _v[2].x(), _v[0].y() - _v[2].y();

        //��2��׼��������������θ������ߵ�����
        Eigen::Vector2f v1;
        v1 << x - _v[0].x(), y - _v[0].y();
        Eigen::Vector2f v2;
        v2 << x - _v[1].x(), y - _v[1].y();
        Eigen::Vector2f v3;
        v3 << x - _v[2].x(), y - _v[2].y();

        //��3�������θ��ߵĵ�������˲�����������θ������ߵ�����
        float z1 = side1.x() * v1.y() - side1.y() * v1.x();
        float z2 = side2.x() * v2.y() - side2.y() * v2.x();
        float z3 = side3.x() * v3.y() - side3.y() * v3.x();

        //��4���жϲ�˽���Ƿ�����ͬ�ķ���
        if ((z1 > 0 && z2 > 0 && z3 > 0) || (z1 < 0 && z2 < 0 && z3 < 0))
        {
            return true;
        }
        else
        {
            return false;
        }
    }


static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
 
    //f12����toVector4()������Է��ַ���ֵ��array<Vector4f, 3>
//�ҵ���⣺v��һ����������������Ϣ�����飬ÿ������������Vector4f��������꣩��ʾ

// TODO : Find out the bounding box of current triangle.
// iterate through the pixel and find if the current pixel is inside the triangle

//��1���þ��ν������ΰ�Χ����,�ҵ����ε��ĸ����㣬���������ΰ�Χ��
    float min_x = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
    float max_x = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
    float min_y = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
    float max_y = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));

    //��2�����������ΰ�Χ���е����в��Ե�
    for (int x = min_x; x <= max_x; x++)
    {
        for (int y = min_y; y <= max_y; y++)
        {
            // If so, use the following code to get the interpolated z value.
            //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            //z_interpolated *= w_reciprocal;

            // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

            if (insideTriangle(x, y, t.v))//�ж��Ƿ�����������,������ڲ�������λ�ô��Ĳ�ֵ���ֵ (interpolated depth value) ����Ȼ����� (depth buffer) �е���Ӧֵ���бȽϡ�
            {//�����Ǽ����ֵ�����ݣ���ʱ������,�ȳ����˲��Ͳ�ȫ
                //��С��ȣ�Ĭ��������Զ
                float min_depth = FLT_MAX;

                //������������ڲ������㵱ǰ���,�õ���ǰ��С���
                auto tup = computeBarycentric2D(x, y, t.v);

                float alpha, beta, gamma;
                std::tie(alpha, beta, gamma) = tup;
                float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                min_depth = std::min(min_depth, z_interpolated);

                //���x,y���ڵ�����С��z-buffer����ȣ�
                if (depth_buf[get_index(x, y)] > min_depth)
                {
                    //������ϲ�Ӧ����Ⱦ����ɫ
                    Vector3f color = t.getColor();
                    Vector3f point;
                    point << x, y, min_depth;
                    //�������
                    depth_buf[get_index(x, y)] = min_depth;
                    //�������ڵ����ɫ
                    set_pixel(point, color);
                }
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on
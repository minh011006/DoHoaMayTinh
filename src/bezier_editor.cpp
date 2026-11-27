#include "bezier_profile.hpp"
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

namespace {
vase::BezierProfile profile;
std::vector<vase::ProfilePoint> samples;
int width=1100, height=720, selected=-1;
std::string csvPath="profile.csv", status;
constexpr double originX=80, originY=630, scale=130;
void label(double x,double y,const std::string& s) {
    glRasterPos2d(x,y);
    for (unsigned char c:s) glutBitmapCharacter(GLUT_BITMAP_9_BY_15,c);
}
double sx(double r) { return originX+r*scale; }
double sy(double y) { return originY-y*scale; }
void refresh(bool save) {
    samples=profile.sample();
    if (save) {
        try { vase::writeProfileCsv(csvPath,samples); status="Saved 101 samples: "+csvPath; }
        catch(const std::exception& e) { status=e.what(); std::cerr<<status<<'\n'; }
    }
    glutPostRedisplay();
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0,1100,720,0,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glColor3f(.12f,.16f,.23f);
    glBegin(GL_QUADS); glVertex2d(550,0); glVertex2d(1100,0); glVertex2d(1100,720); glVertex2d(550,720); glEnd();
    glColor3f(.8f,.85f,.95f);
    label(25,30,"BEZIER PROFILE EDITOR"); label(25,55,"Drag P0-P3 with left mouse button");
    label(25,78,"R: reset | S: save CSV | Esc: quit");
    glLineWidth(1); glColor3f(.22f,.27f,.35f);
    glBegin(GL_LINES);
    for(int i=0;i<=5;++i) { double x=sx(i*.5); glVertex2d(x,110);glVertex2d(x,650); }
    for(int i=0;i<=8;++i) { double y=sy(i*.5);glVertex2d(65,y);glVertex2d(430,y); }
    glEnd();
    glColor3f(.75f,.8f,.9f); glLineWidth(2);
    glBegin(GL_LINES); glVertex2d(originX,110);glVertex2d(originX,650);glVertex2d(65,originY);glVertex2d(440,originY);glEnd();
    label(32,120,"y");label(450,635,"r");label(90,650,"Axis r=0");
    glColor3f(.6f,.62f,.7f); glLineWidth(1); glBegin(GL_LINE_STRIP);
    for(auto p:profile.controls) glVertex2d(sx(p.r),sy(p.y)); glEnd();
    glColor3f(.2f,.9f,.85f);glLineWidth(3);glBegin(GL_LINE_STRIP);
    for(auto p:samples) glVertex2d(sx(p.r),sy(p.y));glEnd();
    for(int i=0;i<4;++i) {
        auto p=profile.controls[i];glColor3f(1, i==selected?.4f:.75f,.2f);
        glBegin(GL_TRIANGLE_FAN);glVertex2d(sx(p.r),sy(p.y));
        for(int j=0;j<=24;++j) { double a=j*6.28318530718/24;glVertex2d(sx(p.r)+8*cos(a),sy(p.y)+8*sin(a)); }glEnd();
        label(sx(p.r)+12,sy(p.y)-10,"P"+std::to_string(i));
    }
    glColor3f(.85f,.9f,1); label(575,30,"OUTPUT (r, y): BASE -> MOUTH");
    label(575,55,"101 samples, ready for generateVaseMesh()");
    for(int i=0;i<4;++i) { std::ostringstream s;s<<std::fixed<<std::setprecision(3)<<"P"<<i<<": ("<<profile.controls[i].r<<", "<<profile.controls[i].y<<")";label(575,95+i*24,s.str()); }
    label(575,210,"Preview: every fifth sample");
    for(int i=0;i<=100;i+=5) {std::ostringstream s;s<<std::setw(3)<<i<<"   "<<std::fixed<<std::setprecision(5)<<samples[i].r<<"   "<<samples[i].y;label(590,238+i/5*19,s.str());}
    label(25,685,"Height order: P0 <= P1 <= P2 <= P3");
    label(575,685,status.substr(0,55)); glutSwapBuffers();
}
void reshape(int w,int h) { width=w>0?w:1;height=h>0?h:1;glViewport(0,0,width,height); }
void drag(int x,int y) {
    if(selected<0)return;
    double px=double(x)*1100/width, py=double(y)*720/height;
    profile.moveControl(selected,(px-originX)/scale,(originY-py)/scale);
    refresh(true);
}
void mouse(int button,int state,int x,int y) {
    if(button!=GLUT_LEFT_BUTTON)return;
    if(state==GLUT_UP) { selected=-1;glutPostRedisplay();return; }
    double px=double(x)*1100/width, py=double(y)*720/height;
    if(px>=550)return;
    double best=14*14;selected=-1;
    for(int i=0;i<4;++i) {auto p=profile.controls[i];double d=std::pow(px-sx(p.r),2)+std::pow(py-sy(p.y),2);if(d<=best){best=d;selected=i;}}
    glutPostRedisplay();
}
void keyboard(unsigned char k,int,int) {
    if(k==27)glutLeaveMainLoop();
    if(k=='r'||k=='R'){profile=vase::BezierProfile{};selected=-1;refresh(true);}
    if(k=='s'||k=='S')refresh(true);
}
}
int main(int argc,char** argv) {
    for(int i=1;i<argc;++i)if(std::string(argv[i])=="--output" && i+1<argc)csvPath=argv[++i];
    glutInit(&argc,argv);glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width,height);glutCreateWindow("Bezier profile - DoHoaMayTinh");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glClearColor(.065f,.085f,.13f,1);
    glutDisplayFunc(display);glutReshapeFunc(reshape);glutMouseFunc(mouse);glutMotionFunc(drag);glutKeyboardFunc(keyboard);
    refresh(true);glutMainLoop();
}

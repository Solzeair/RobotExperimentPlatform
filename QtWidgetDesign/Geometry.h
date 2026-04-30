#ifndef _analytic_geometry
#define _analytic_geometry
#include "Globe.h"
#include "F2C.H"
typedef struct tagLineFormulation{
	double a;
	double b;
	double c;
}Formulation, LINEFORMULATION;
typedef struct tagCircleFormulation{
	double x;
	double y;
	double r;
}CircleFormulation, CIRCLEFORMULATION;

/********* chen ning **************/
int CalcuFormat(POINT P2, POINT P1, Formulation *Param);//aY=bX+c 
int gmFormulate(POINT P1, POINT P2, Formulation *FormuParam);//aX+bY+c=0
int StdLineForm(dbPOINT P1, dbPOINT P2, Formulation *FormuParam);//aX+bY+c=0
int StdLineForm(dbPOINT point, double angle, LINEFORMULATION *Result);//点斜式得出标准方程
int cn_PointSlopeLine(dbPOINT point, double angle, LINEFORMULATION *Result);//点斜式得出标准方程
int cn_2LinesCrossPoint(Formulation *Line1, Formulation *Line2, dbPOINT *Result);//两直线交点
int cn_2LinesCrossPoint(dbPOINT A1, dbPOINT A2, dbPOINT B1, dbPOINT B2, dbPOINT *Result);
int cn_2SectionsCross(POINT A, POINT B, POINT C, POINT D, POINT* P);//obslete
int cn_2SectionsCross(dbPOINT A, dbPOINT B, dbPOINT C, dbPOINT D, dbPOINT* P);//两线段交点
int cn_LineCircleCross(LINEFORMULATION *pLine, CIRCLEFORMULATION *pCircle, dbPOINT *Point1, dbPOINT *Point2);//直线和圆的交点
int cn_LineCircleCross(dbPOINT A, dbPOINT B, CIRCLEFORMULATION *pCircle, dbPOINT *Point1, dbPOINT *Point2);//直线和圆的交点
double cn_LineAngle(LINEFORMULATION *pLine);//直线与x轴的夹角
double cn_LineAngle(dbPOINT A, dbPOINT B);//直线与x轴的夹角
double cn_LineAngle(POINT A, POINT B);
int cn_PointPerpendLine(dbPOINT Point, LINEFORMULATION *pLine, LINEFORMULATION *pResult, dbPOINT *pPointCross);//过一点垂直于一条直线的方程和交点
int cn_PointPerpendLine(dbPOINT Point, dbPOINT A, dbPOINT B, LINEFORMULATION *pResult, dbPOINT *pPointCross);//过一点垂直于一条直线的方程和交点
int cn_DistanceLines(LINEFORMULATION *pRefLine, double Distance, LINEFORMULATION *Result1, LINEFORMULATION *Result2);//与一条直线相距一定距离的直线方程
int cn_2CirclesCross(CIRCLEFORMULATION *pCircle1, CIRCLEFORMULATION *pCircle2, dbPOINT *Point1, dbPOINT *Point2);//两圆的交点
double cn_PointLineDist(POINT P, POINT A, POINT B);//点到直线的距离
double cn_PointLineDist(dbPOINT P, dbPOINT A, dbPOINT B);//点到直线的距离
double cn_PointLineDist(dbPOINT Point, LINEFORMULATION *Line);//点到直线的距离
double cn_2PointsDist(dbPOINT A, dbPOINT B);//两点间距离
double distRobot2Pt(RobotInford robot,dbPOINT point);//车到点的距离
double dist2Robot(RobotInford robot1, RobotInford robot2); //两车间距离
double cn_2PointsDist(POINT A, POINT B);
double cn_AngleTrim2PI(double theta);//变化角度到2*pi
double cn_AngleTrimPI(double theta);//变化角度到pi
int cn_IsPointInSegment(dbPOINT Pt, dbPOINT A, dbPOINT B);//判断点是否在线段内
int cn_IsPointInArc(dbPOINT Pt, CIRCLEFORMULATION *pCircleFormulation, double sTheta, double dTheta);//判断点是否在弧内
int cn_ArcSegmentCross( CIRCLEFORMULATION *pCircleFormulation, double sTheta, double dTheta, dbPOINT A, dbPOINT B, dbPOINT *pResultA, dbPOINT *pResultB);//弧与线段的交点
double cn_RefractAngle(POINT A, POINT B, POINT C, POINT D);
double cn_RefractAngle(dbPOINT A, dbPOINT B, dbPOINT C, dbPOINT D);
double cn_2PointPerpendLine(dbPOINT A, dbPOINT B, LINEFORMULATION *pResult, dbPOINT *pPointCross);
int cn_2Points_a_equalPoint(dbPOINT A, dbPOINT B, double a, dbPOINT C);
int cn_PointToLineDistance(dbPOINT A,LINEFORMULATION *Line,double d);
 
int IsInVectorRegion(POINT A, POINT P1, POINT P2);
int IsInObject(POINT A, POINT *points, int number);
int IsInObject(POINT A, POINT O, int radian);
int gmIsInVectorRegion(POINT A, POINT P1, POINT P2);
int cn_VectorRegion(dbPOINT A, dbPOINT P1, dbPOINT P2);
int cn_VectorRegion(POINT A, POINT P1, POINT P2);
int cn_PointInLineSegment(dbPOINT A, dbPOINT B, double Distance, dbPOINT *Result);
int cn_PointDistanceInLine(dbPOINT A, dbPOINT B, double Distance, dbPOINT *Result1, dbPOINT *Result2);
int cn_LineSegmentHoldin(dbPOINT P, dbPOINT A, dbPOINT B);
void CoordinateTransform(dbROBOTPOSTURE *pNewCoordiante, dbPOINT Target, dbPOINT *Result);
int cn_SegmentBreak(SEGMENT *pSegment, dbPOINT SrcPoint, dbPOINT ObtPoint, double Radius,  SEGMENT *pSegment1, SEGMENT *pSegment2);//求出无障碍通道
int cn_SegmentBreak(SEGMENT *pSegment, int Number, dbPOINT SrcPoint, dbPOINT ObtPoint, double Radius);//求出无障碍通道
/*********** end ************/
/*******************q求解线性方程组－－wtq------------------*/
int gmiv(double a[],int m,int n,double b[],double x[],double aa[],double eps,double u[],double v[],int ka);
 int ginv(double a[],int m,int n,double aa[],double eps,double u[],double v[],int ka);
  static void sss(double fg[],double cs[]);
  static void ppp(double a[],double e[],double s[],double v[],int m,int n);
  int muav(double a[],int m,int n,double u[],double v[],double eps,int ka);
/************ zhou yunlong **************/
int MiddleVerticalLine(dbPOINT point1, dbPOINT point2,Formulation *Formu);
int PointToPointDirectionAngle(dbPOINT Point1,dbPOINT Point2,double *pAngle);
/************ end *******************/

/********************非线性方程组参数优化----dou_yd----------------*/
void lmfunc1(integer *,integer *,doublereal *,doublereal *,integer *);
void lmfunc2(integer *,integer *,doublereal *,doublereal *,integer *);
void lmfunc3(integer *,integer *,doublereal *,doublereal *,integer *);
void lmfunc4(integer *,integer *,doublereal *,doublereal *,integer *);
void lmfunc5(integer *,integer *,doublereal *,doublereal *,integer *);
doublereal dpmpar_(integer *i);
doublereal enorm_(integer *n, doublereal *x);
int qrfac_(integer *m,integer *n, doublereal *a, integer *lda, logical *pivot, integer *ipvt, integer *lipvt, doublereal *rdiag, doublereal *acnorm, doublereal *wa);
int qrsolv_(integer *n, doublereal *r, integer *ldr, integer *ipvt, doublereal *diag, doublereal *qtb, doublereal *x, doublereal *sdiag, doublereal *wa);
int lmpar_(integer *n,doublereal * r, integer *ldr, integer *ipvt, doublereal *diag, doublereal *qtb, doublereal *delta, doublereal *par,doublereal * x, doublereal *sdiag, doublereal *wa1, doublereal *wa2);
int fdjac2_(void (*fcn) (integer *,integer *, doublereal *,doublereal *,integer *),
			integer * m, integer *n, doublereal *x, doublereal *fvec, doublereal *fjac, 
			integer *ldfjac, integer *iflag, doublereal *epsfcn, doublereal *wa);
int lmdif_(void (*fcn) (integer *,integer *, doublereal *,doublereal *,integer *), 
    integer *m, integer *n, doublereal *x, doublereal *fvec, doublereal *ftol, 
	doublereal *xtol, doublereal *gtol, integer *maxfev, doublereal *epsfcn, doublereal *diag, 
	integer *mode, doublereal *factor, integer *nprint, integer *info, integer *nfev, 
	doublereal *fjac, integer *ldfjac, integer *ipvt, doublereal *qtf,doublereal * wa1,doublereal * wa2, doublereal *wa3, doublereal *wa4);
/************ end *******************/
#endif

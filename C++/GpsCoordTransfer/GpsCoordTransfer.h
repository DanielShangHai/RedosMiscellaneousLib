#pragma once

#define C_54_SYSTEM 1
#define C_84_SYSTEM 2

namespace gpscord
{
	 const double _PIvalue = 3.1415926535897932384626433832795;
     const double _Precision = 1e-20; // 判断double是否为零的精度标准
     const double _LongAXIS = 6378137.0;//84长轴
     const double _ECC = 0.00669437999014132;//45扁率
     const double earth_a = 6378137;//84
     const double earth_squaree = 0.00669437999013;// 84扁率
    // const double earth_squaree1 = 0.00673949674227;// 84扁率


class GpsCoordTransfer
{
public:
	GpsCoordTransfer(void);
	~GpsCoordTransfer(void);
private:
	int m_CoordinateSystemSel;

	// 4参数
    double m_shiftX0;
    double m_shiftY0;
    double m_rotateAngle;//顺时钟旋转角
    double m_zoomScale;  //尺度

	// 中央子午线
    double m_CenterLine;
    



public:
	bool BLH2XYZ(double B, double L, double H, double& X, double& Y, double& Z);
	void TransBLH(double B, double L, double H, double& X, double& Y, double& Z);
	void BLToXY(double StationB, double StationL, double& StationPx, double& StationPy, double AreaCentLong, int cordsysSel);
private:
	static double Get_atan(double z, double y);
public:
	void OnXYZToxyh(double X, double Y, double Z, double centerLine, int cordSysflag, double& retX, double& retY, double& retH);
private:
	void ToLocalCoord(double& X, double& Y);
public:
	void setTransferParameter(double shiftX, double shiftY, double rotateAngle, double Kscale);
	void getTransferParameter(double& shiftX, double& shiftY, double& rotateAngle, double& Kscale);
	void setCoordSystemSel(int sel);
	int getCoordSystemSel(void);
};
}
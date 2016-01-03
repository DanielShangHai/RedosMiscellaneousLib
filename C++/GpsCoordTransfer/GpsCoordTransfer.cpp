#include "GpsCoordTransfer.h"
#include "math.h"


namespace gpscord
{


GpsCoordTransfer::GpsCoordTransfer(void)
: m_CoordinateSystemSel(0)
, m_CenterLine(0)
{
	m_CoordinateSystemSel = C_54_SYSTEM;
    m_shiftX0 = 0.0;
    m_shiftY0 = 0.0;
    m_rotateAngle = 0.0;
    m_zoomScale = 1.0;
	// 中央子午线
    m_CenterLine = 117.0;
}

GpsCoordTransfer::~GpsCoordTransfer(void)
{
}

bool GpsCoordTransfer::BLH2XYZ(double B, double L, double H, double& X, double& Y, double& Z)
{
	double x0, y0, z0;
    TransBLH(B * (_PIvalue / 180), L * (_PIvalue / 180), H, x0, y0, z0);
    OnXYZToxyh(x0, y0, z0, m_CenterLine, m_CoordinateSystemSel, X, Y, Z);
    ToLocalCoord(X, Y);
	return true;
}

void GpsCoordTransfer::TransBLH(double B, double L, double H, double& X, double& Y, double& Z)
{
	double cosB = cos(B);
    double cosL = cos(L);  //C# 里的三角函数参数是弧度 和 C++同
    double sinL = sin(L);
    double sinB = sin(B);
    double N = _LongAXIS / sqrt(1.0 - _ECC * sinB * sinB);
    X = (N + H) * cosB * cosL;
    Y = (N + H) * cosB * sinL;
    Z = sinB * (N * (1 - _ECC) + H);
}

void GpsCoordTransfer::BLToXY(double StationB, double StationL, double& StationPx, double& StationPy, double AreaCentLong, int cordsysSel)
{
	        double A = 6378137;
            double B = 6356752.3142;
            double AAS = A * A;
            double BBS = B * B;
            double ES = 0.00669437999013;
            double E1S = 0.00673949674227;
            double a;

            if (cordsysSel == C_54_SYSTEM)//选取54z坐标
            {
                A = 6378245;       //椭球长半径
                B = 6356863;    //椭球短半径
                AAS = A * A;
                BBS = B * B;
                a = 1 / 298.3;    //  扁率
                ES = 2 * a - a * a;    //第一偏心率  
                E1S = (A * A) / (B * B) - 1;   //第二偏心率平方
            }
            double E2S = ES * ES;
            double E3S = ES * E2S;
            double E4S = E2S * E2S;
            double E5S = E2S * E3S;
            double E6S = E3S * E3S;
            double KX = A * (1 - ES);
            double KA1 = 1 + 3 * ES / 4 + 45 * E2S / 64 + 175 * E3S / 256 + 11025 * E4S / 16384;
            double KA = KA1 + 43659 * E5S / 65536 + 693693 * E6S / 1048576;
            double KB = KA - 1;
            double KC = 15 * E2S / 32 + 175 * E3S / 384 + 3675 * E4S / 8192 + 14553 * E5S / 32768 + 231231 * E6S / 524288;
            double KD = 35 * E3S / 96 + 735 * E4S / 2048 + 14553 * E5S / 40960 + 231231 * E6S / 655360;
            double KE = 315 * E4S / 1024 + 6237 * E5S / 20480 + 99099 * E6S / 327680;
            double KF = 693 * E5S / 2560 + 11011 * E6S / 40960;
            double KG = 1001 * E6S / 4096;

            StationB = StationB * _PIvalue / 180;
            StationL = StationL * _PIvalue / 180;
            AreaCentLong = AreaCentLong * _PIvalue / 180;

            double SC = sin(StationB) * cos(StationB);
            double SS = sin(StationB) * sin(StationB);
            double S2S = SS * SS;
            double S3S = SS * S2S;
            double XA0 = KX * KA * StationB - KX * SC * (KB + SS * (KC + SS * (KD + KE * SS + KF * S2S + KG * S3S)));
            double T = tan(StationB);
            double StationLd = StationL - AreaCentLong;
            double TS = T * T;
            double T2S = TS * TS;
            double T3S = TS * T2S;
            double CS = cos(StationB) * cos(StationB);
            double NR = AAS / sqrt(AAS * CS + BBS * SS);
            double NAS = E1S * CS;
            double N2S = NAS * NAS;
            double LDS = StationLd * StationLd;
            double FA = NR * SC * LDS;
            double FB = CS * LDS;
            double FC = NR * cos(StationB) * StationLd;
            double FD = (5 - TS + 9 * NAS + 4 * N2S) / 24;
            double FF = (61 - 58 * TS + T2S + 270 * NAS - 330 * NAS * TS) / 720;
            double FG = (1385 - 3111 * TS + 543 * T2S - T3S) / 40320;
            double FH = (1 - TS + NAS) / 6;
            double FI = (5 - 18 * TS + T2S + 14 * NAS - 58 * NAS * TS) / 120;
            double FJ = (61 - 479 * TS + 179 * T2S - T3S) / 5040;
            StationPx = XA0 + FA * (0.5 + FB * (FD + FB * (FF + FB * FG)));
            StationPy = FC * (1 + FB * (FH + FB * (FI + FB * FJ)));
            StationPy = StationPy + 500000;
}

double GpsCoordTransfer::Get_atan(double z, double y)
{
    double x;
	if (abs(z) < _Precision) // z == 0
		x = _PIvalue / 2;
	else if (abs(y) < _Precision) // y == 0
		x = _PIvalue;
	else
	{
		x = atan(abs(y / z));

		if ((y > 0) && (z < 0))
			x = _PIvalue - x;
		else if ((y < 0) && (z < 0))
			x = _PIvalue + x;
		else if ((y < 0) && (z > 0))
			x = 2 * _PIvalue - x;
	}

	return x;
}

void GpsCoordTransfer::OnXYZToxyh(double X, double Y, double Z, double centerLine, int cordSysflag, double& retX, double& retY, double& retH)
{

    double B;
    double B0;
    double L;
    double H;
    double N;

    double Px;
    double Py;

    if (cordSysflag != C_54_SYSTEM) //54坐标系
    {
        ;
    }
    //XYZ至BLH
    L = Get_atan(X, Y);
    B = 0.68;
    do
    {
        B0 = B;
        N = earth_a / sqrt(1 - earth_squaree * sin(B0) * sin(B0));
        H = Z / sin(B0) - N * (1 - earth_squaree);

        B = Get_atan(sqrt(X * X + Y * Y) * (N * (1 - earth_squaree) + H), Z * (N + H));

    } while (abs(B - B0) > 0.000001);

    H = Z / sin(B) - N * (1 - earth_squaree);

    //BLH至xy(注:各地区的中央子午线各异)
    BLToXY(B / _PIvalue * 180, L / _PIvalue * 180, Px, Py, centerLine, cordSysflag);//中央子午线  117

    retX = Px;
    retY = Py;
    retH = H;
}

void GpsCoordTransfer::ToLocalCoord(double& X, double& Y)
{
	double x1 = m_shiftX0 + (X * cos(m_rotateAngle) - Y * sin(m_rotateAngle)) * m_zoomScale;
    double y1 = m_shiftY0 + (Y * cos(m_rotateAngle) + X * sin(m_rotateAngle)) * m_zoomScale;
    X = x1;
    Y = y1;
}


}
void gpscord::GpsCoordTransfer::setTransferParameter(double shiftX, double shiftY, double rotateAngle, double Kscale)
{
	//m_CoordinateSystemSel;
    m_shiftX0 = shiftX;
    m_shiftY0 = shiftY;
    m_rotateAngle = rotateAngle;
    m_zoomScale = Kscale;
}

void gpscord::GpsCoordTransfer::getTransferParameter(double& shiftX, double& shiftY, double& rotateAngle, double& Kscale)
{
	shiftX = m_shiftX0;
    shiftY = m_shiftY0;
    rotateAngle = m_rotateAngle;
    Kscale = m_zoomScale;
}

void gpscord::GpsCoordTransfer::setCoordSystemSel(int sel)
{
	m_CoordinateSystemSel = sel;
}

int gpscord::GpsCoordTransfer::getCoordSystemSel(void)
{
	return m_CoordinateSystemSel;
}

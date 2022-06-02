#include <opencv2/opencv.hpp>
#include <raspicam_cv.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace raspicam;

Mat frame, Matrix, framePers, frameGray, frameThresh, frameEdge, frameFinal, frameFinalDuplicate, frameFinalDuplicate1;
Mat ROILane, ROILaneEnd;
int LeftLanePos, RightLanePos, frameCenter, laneCenter, Result, laneEnd, redResult ,greenResult;

//This matric is additional ! 
Mat frameRed ,framePersHV ,frameGreen;
Mat ROIRed ,ROIGreen;
vector<int> histrogramRedColor;
vector<int> histrogramGreenColor;
int laneRed ,lanGreen;
Mat frameFinalDuplicate3, frameFinalDuplicate4;
RaspiCam_Cv Camera;

stringstream ss;


vector<int> histrogramLane;
vector<int> histrogramLaneEnd;

Point2f Source[] = {Point2f(70,180),Point2f(250,180),Point2f(30,240), Point2f(300,240)};
Point2f Destination[] = {Point2f(100,0),Point2f(280,0),Point2f(100,240), Point2f(280,240)};


 void Setup ( int argc,char **argv, RaspiCam_Cv &Camera )
  {
    Camera.set ( CAP_PROP_FRAME_WIDTH,  ( "-w",argc,argv,400 ) );
    Camera.set ( CAP_PROP_FRAME_HEIGHT,  ( "-h",argc,argv,240 ) );
    Camera.set ( CAP_PROP_BRIGHTNESS, ( "-br",argc,argv,50 ) );
    Camera.set ( CAP_PROP_CONTRAST ,( "-co",argc,argv,55 ) );
    Camera.set ( CAP_PROP_SATURATION,  ( "-sa",argc,argv,50 ) );
    Camera.set ( CAP_PROP_GAIN,  ( "-g",argc,argv ,50 ) );
    Camera.set ( CAP_PROP_FPS,  ( "-fps",argc,argv,0));

}

void Capture()
{
    Camera.grab();
    Camera.retrieve( frame);
    cvtColor(frame, frame, COLOR_BGR2RGB);
}

void Perspective()
{
	line(frame,Source[0], Source[1], Scalar(0,0,255), 2);
	line(frame,Source[1], Source[3], Scalar(0,0,255), 2);
	line(frame,Source[3], Source[2], Scalar(0,0,255), 2);
	line(frame,Source[2], Source[0], Scalar(0,0,255), 2);
	
	
	Matrix = getPerspectiveTransform(Source, Destination);
	warpPerspective(frame, framePers, Matrix, Size(400,240));
}

void Threshold()
{
	cvtColor(framePers, frameGray, COLOR_RGB2GRAY);
	//cvtColor(framePers ,framePersHV ,COLOR_BGR2HSV);
	cvtColor(frame ,framePersHV ,COLOR_BGR2HSV);
	inRange(frameGray, 230, 255, frameThresh);
	//inRange(framePersHV ,Scalar(170, 70, 50), Scalar(180, 255, 255) ,frameRed);
	inRange(framePersHV ,Scalar(159, 50, 70), Scalar(180, 255, 255) ,frameRed);
	inRange(framePersHV ,Scalar(36, 50, 70), Scalar(89, 255, 255) ,frameGreen);
	Canny(frameGray,frameEdge, 900, 900, 3, false);
	add(frameThresh, frameEdge, frameFinal);
	cvtColor(frameFinal, frameFinal, COLOR_GRAY2RGB);
	cvtColor(frameFinal, frameFinalDuplicate, COLOR_RGB2BGR);   //used in histrogram function only
	cvtColor(frameFinal, frameFinalDuplicate1, COLOR_RGB2BGR);   //used in histrogram function only
	cvtColor(frameRed ,frameFinalDuplicate3 ,COLOR_RGB2BGR);
	cvtColor(frameGreen ,frameFinalDuplicate4 ,COLOR_RGB2BGR);

	
}

void Histrogram()
{
    histrogramLane.resize(300);
    histrogramLane.clear();
    
    for(int i=0; i<300; i++)       //frame.size().width = 400
    {
	ROILane = frameFinalDuplicate(Rect(i,140,1,100));
	divide(255, ROILane, ROILane);
	histrogramLane.push_back((int)(sum(ROILane)[0])); 
    }
	
	histrogramLaneEnd.resize(300);
        histrogramLaneEnd.clear();
	for (int i = 0; i < 300; i++)       
	{
		ROILaneEnd = frameFinalDuplicate1(Rect(i, 0, 1, 240));   
		divide(255, ROILaneEnd, ROILaneEnd);       
		histrogramLaneEnd.push_back((int)(sum(ROILaneEnd)[0]));  
		
	
	}
	   laneEnd = sum(histrogramLaneEnd)[0];
	   cout<<"Lane END = "<<laneEnd<<endl;
    
    histrogramRedColor.resize(400);
    histrogramRedColor.clear();
	for (int i = 0; i < 400; i++)       
	{
		ROIRed = frameFinalDuplicate3(Rect(i, 0, 1, 240));   
		divide(255, ROIRed, ROIRed);       
		histrogramRedColor.push_back((int)(sum(ROIRed)[0]));  
	}
	laneRed = sum(histrogramRedColor)[0];
	cout<<"red is heeeeeeere = "<<laneRed<<endl;
	
    histrogramGreenColor.resize(400);
    histrogramGreenColor.clear();
	for (int i = 0; i < 400; i++)       
	{
		ROIGreen = frameFinalDuplicate4(Rect(i, 0, 1, 240));   
		divide(255, ROIGreen, ROIGreen);       
		histrogramGreenColor.push_back((int)(sum(ROIGreen)[0]));  
	}
	lanGreen = sum(histrogramGreenColor)[0];
	cout<<"green is heeeeeeere = "<<lanGreen<<endl;
	
}

void LaneFinder()
{
    vector<int>:: iterator LeftPtr;
    LeftPtr = max_element(histrogramLane.begin(), histrogramLane.begin() + 160);
    LeftLanePos = distance(histrogramLane.begin(), LeftPtr); 
    
    vector<int>:: iterator RightPtr;
    RightPtr = max_element(histrogramLane.begin() +280, histrogramLane.end());
    RightLanePos = distance(histrogramLane.begin(), RightPtr);
    
    line(frameFinal, Point2f(LeftLanePos, 0), Point2f(LeftLanePos, 240), Scalar(0, 255,0), 2);
    line(frameFinal, Point2f(RightLanePos, 0), Point2f(RightLanePos, 240), Scalar(0,255,0), 2); 
}

void LaneCenter()
{
    laneCenter = (RightLanePos-LeftLanePos)/2 +LeftLanePos;
    frameCenter = 188;
    
    line(frameFinal, Point2f(laneCenter,0), Point2f(laneCenter,240), Scalar(0,255,0), 3);
    line(frameFinal, Point2f(frameCenter,0), Point2f(frameCenter,240), Scalar(255,0,0), 3);

    Result = laneCenter-frameCenter;
}


int main(int argc,char **argv)
{
	
    wiringPiSetup();
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    
    Setup(argc, argv, Camera);
    cout<<"Connecting to camera"<<endl;
    if (!Camera.open())
    {
		
	cout<<"Failed to Connect"<<endl;
    }
     
	cout<<"Camera Id = "<<Camera.getId()<<endl;
     
 
    while(1)
    {
	
    auto start = std::chrono::system_clock::now();

    Capture();
    Perspective();
    Threshold();
    Histrogram();
    LaneFinder();
    LaneCenter();
    
    if (laneEnd > 3000)
    {
       	digitalWrite(21, 1);
	digitalWrite(22, 1);    //decimal = 7
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Lane End"<<endl;
    }
    
    
    if (Result == 0)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 0);    //decimal = 14
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Forward"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        
    else if (Result >0 && Result <10)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 0);    //decimal = 1
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right1"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        else if (Result >=10 && Result <20)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 2
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right2"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        else if (Result >20)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 1);    //decimal = 3
	digitalWrite(23, 0);
	digitalWrite(24, 0);
	cout<<"Right3"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        else if (Result <0 && Result >-10)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 0);    //decimal = 4
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left1"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        else if (Result <=-10 && Result >-20)
    {
	digitalWrite(21, 1);
	digitalWrite(22, 0);    //decimal = 5
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left2"<<laneCenter<<","<<frameCenter<<endl;
    }
    
        else if (Result <-20)
    {
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 6
	digitalWrite(23, 1);
	digitalWrite(24, 0);
	cout<<"Left3"<<laneCenter<<","<<frameCenter<<endl;
    }
    if (lanGreen > 3000){
	digitalWrite(21, 0);
	digitalWrite(22, 1);    //decimal = 10
	digitalWrite(23, 0);
	digitalWrite(24, 1);
	cout<<"it's Green !! I can Park HEEEERE !!! "<<endl;
    }
	
	
    
   if (laneEnd > 10000)
    {
       ss.str(" ");
       ss.clear();
       ss<<" Lane End";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(255,0,0), 2);
    
     }
    
    else if (Result == 0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<" Move Forward";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    
     }
    
    else if (Result > 0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<"bMove Right";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    
     }
     
     else if (Result < 0)
    {
       ss.str(" ");
       ss.clear();
       ss<<"Result = "<<Result<<" Move Left";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(0,0,255), 2);
    
     }
     
     if (laneRed > 10000){
       ss.str(" ");
       ss.clear();
       ss<<" Parking is full ! :'";
       putText(frame, ss.str(), Point2f(5,50), 0,1, Scalar(255,0,0), 2);
    
     }
     
     else if (lanGreen > 3000){
       ss.str(" ");
       ss.clear();
       ss<<" Parking is free HAHAAH ! :'";
       putText(frame, ss.str(), Point2f(1,50), 0,1, Scalar(255,0,0), 2);
    
     }
    
    
    namedWindow("orignal", WINDOW_KEEPRATIO);
    moveWindow("orignal", 0, 100);
    resizeWindow("orignal", 640, 480);
    imshow("orignal", frame);
    
    namedWindow("Perspective", WINDOW_KEEPRATIO);
    moveWindow("Perspective", 640, 100);
    resizeWindow("Perspective", 640, 480);
    imshow("Perspective", framePers);
    
    namedWindow("Final", WINDOW_KEEPRATIO);
    moveWindow("Final", 1280, 100);
    resizeWindow("Final", 640, 480);
    imshow("Final", frameFinal);
    
    namedWindow("Red", WINDOW_KEEPRATIO);
    moveWindow("Red", 1280, 400);
    resizeWindow("Red", 640, 480);
    imshow("Red", frameRed);
    
    namedWindow("Green", WINDOW_KEEPRATIO);
    moveWindow("Green", 1280, 700);
    resizeWindow("Green", 640, 480);
    imshow("Green", frameGreen);
    
    waitKey(1);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    
    float t = elapsed_seconds.count();
    int FPS = 1/t;
    //cout<<"FPS = "<<FPS<<endl;
    
    }

    
    return 0;
     
}

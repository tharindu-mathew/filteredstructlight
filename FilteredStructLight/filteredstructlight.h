#ifndef FILTEREDSTRUCTLIGHT_H
#define FILTEREDSTRUCTLIGHT_H

#include "cameradisplaywidget.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "camthread.h"
#include "reconstruct.h"

#define CAM_CALIB_PAIRS 6
class FilteredStructLight : public QMainWindow
{
	Q_OBJECT

public:
	FilteredStructLight(QWidget *parent = 0);
	~FilteredStructLight();
	void setupUi();

protected:
	void keyReleaseEvent(QKeyEvent* e);
	void onClose(QKeyEvent* e);

private:
	CamThread* cam_thread_;
	Reconstruct3D* reconstructor_;

	QWidget* central_widget_;

	QWidget* left_panel_;
	QPushButton* view_cameras_;

	QWidget* right_panel_;
	GLWidget* opengl_widget_;

	QSlider* threshold_slider_;
	QCheckBox* threshold_toggle_checkbox_;

	QGroupBox* camera_pairs_group_;
	QSpinBox* camera_pair_[CAM_CALIB_PAIRS];

	QGroupBox* calibration_group_;
	QPushButton* start_calibration_video_;
	QPushButton* end_calibration_video_;

	QGroupBox* reconstruction_group_;
	QPushButton* load_camera_calibration_;
	QPushButton* start_reconstruction_video_;
	QPushButton* end_reconstruction_video_;
	QPushButton* recalibrate_button;

	void shutdown_cam_thread();
	void create_camera_pairs(CameraPairs& pairs);
};

#endif // FILTEREDSTRUCTLIGHT_H

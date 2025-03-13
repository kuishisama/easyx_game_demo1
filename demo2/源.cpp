#include<graphics.h>
#include<iostream>
#include<string>
#include<vector>
#include <windows.h> 
#include <chrono>
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib,"Winmm.lib")
using namespace std::chrono;

int idx_current_anim = 0; //����֡����
const int PLAYER_ANIM_NUM = 6;
//�ƻ�ʹ��һ��int������ʾ��Ϸ����
int gamestatus = 0;

steady_clock::time_point lastDeathTime;  // ��¼boss����ʱ��

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

//IMAGE img_player_left[PLAYER_ANIM_NUM];
//IMAGE img_player_right[PLAYER_ANIM_NUM];

POINT player_pos = {500,500};


//��Ϸ�������
bool is_game_started = false;
bool running = true;
bool is_game_over = false;
bool is_dark = false; //�䰵�׶�
bool is_white = false; //�����׶�


inline void putimage_alpha(int x,int y,IMAGE*img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });

}

//͸�����ƺ���
/*
void putpicture(int dstx, int dsty, IMAGE* img, COLORREF color, int alpha) {
	DWORD* imgp = GetImageBuffer(img);  // ��ȡԴͼ�����������
	int w = img->getwidth();            // ��ȡԴͼ��Ŀ��
	int h = img->getheight();           // ��ȡԴͼ��ĸ߶�
	int bw = getwidth();                // ��ȡĿ��ͼ��Ŀ��
	DWORD* bgimgp = GetImageBuffer();   // ��ȡĿ��ͼ�����������

	if (bgimgp == nullptr) {
		std::cerr << "Error: bgimgp is null!" << std::endl;
		return;
	}

	color += 0xff000000;  // ���� alpha ͨ����ȷ����ɫ�������ģ�����͸���ȣ�

	// ȷ�� alpha ֵ�� 0 �� 255 ֮��
	if (alpha < 0) alpha = 0;
	else if (alpha > 255) alpha = 255;

	// ѭ������Դͼ���ÿһ������
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			// �ж������Ƿ�Ϊ͸������ɫ
			if (imgp[i * w + j] != color) {
				// ��ȡԴͼ��ǰ���ص� RGB ����
				int r = GetRValue(imgp[i * w + j]);
				int g = GetGValue(imgp[i * w + j]);
				int b = GetBValue(imgp[i * w + j]);

				// �����Ϻ��͸����
				r = (int)(alpha / 255.0 * r + (1 - alpha / 255.0) * GetRValue(color));
				g = (int)(alpha / 255.0 * g + (1 - alpha / 255.0) * GetGValue(color));
				b = (int)(alpha / 255.0 * b + (1 - alpha / 255.0) * GetBValue(color));

				// ȷ��Ŀ��ͼ��λ�ò�Խ��
				if ((i + dsty) >= 0 && (i + dsty) < h && (j + dstx) >= 0 && (j + dstx) < bw) {
					// ������������ֵд��Ŀ��ͼ��
					bgimgp[(i + dsty) * bw + j + dstx] = RGB(r, g, b);
				}
			}
		}
	}
}
*/

class Atlas
{
public:
	Atlas(LPCTSTR path, int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);  // ����֡ͼ��
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		// �����ڴ�
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}
public:
	std::vector<IMAGE*> frame_list;            // �洢֡ͼ����б�

};

class Animation
{
public:
	Animation(Atlas* atlas,int interval) //·�������������
	{
		anim_atlas = atlas;
		interval_ms = interval;

	}

	~Animation() = default;
	
void Play(int x, int y, int delta)//��ʱ�����ƶ���֡
{
	timer += delta;
	if (timer >= interval_ms)
	{
		idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
		timer = 0;
	}
	putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
}

private:
	int timer = 0; //������ʱ��
	int idx_frame = 0;//����֡����
	int interval_ms = 0;
	
private:
	Atlas* anim_atlas;
};


Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_MeleeEnemy_left;
Atlas* atlas_MeleeEnemy_right;
Atlas* atlas_RangedEnemy_left;
Atlas* atlas_RangedEnemy_right;
Atlas* atlas_SpringEnemy_left;
Atlas* atlas_SpringEnemy_right;
Atlas* atlas_player_left_w;
Atlas* atlas_player_right_w;
/*
void LoadAnimation() {
	atlas_player_left = new Atlas(L"img/paimon_left_%d.png", PLAYER_ANIM_NUM);
	atlas_player_right = new Atlas(L"img/paimon_right_%d.png", PLAYER_ANIM_NUM);
}
*/

//�����
class Player
{
public:
	
	Player() : Player_HP(100),Player_MP(0) // ��ʼ�� Player_HP Ϊ 100 MPΪ0
	{
		//������Ӱ
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		loadimage(&img_F_btn, _T("img/F_btn_pushed.png"));
		loadimage(&img_f_btn, _T("img/F_btn_released.png"));
		anim_left = new Animation(atlas_player_left, 45);
		anim_right = new Animation(atlas_player_right, 45);
		anim_left_w = new Animation(atlas_player_left_w, 45);
		anim_right_w = new Animation(atlas_player_right_w, 45);
		
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
		delete anim_left_w;
		delete anim_right_w;
	}

	//������ҵĲ�����Ϣ
	void ProcessEvent(const ExMessage& msg)
	{
		
			 
		     switch (msg.message)
			{
			 case WM_KEYDOWN:
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = true;
					break;
				case VK_DOWN:
					is_move_down = true;
					break;
				case VK_LEFT:
					is_move_left = true;
					break;
				case VK_RIGHT:
					is_move_right = true;
					break;
				case 'F': // ��� 'F' ��
					skillready = true;
					break;
				}
				break;
			
			 case WM_KEYUP:
			
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = false;
					break;
				case VK_DOWN:
					is_move_down = false;
					break;
				case VK_LEFT:
					is_move_left = false;
					break;
				case VK_RIGHT:
					is_move_right = false;
					break;
				}
				break;
			}
		
	}
	//����ƶ�
	void Move()
	{
		//�ٶȳ��Է���������֤�ٶ�λ����ͬ
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(PLAYER_SPEED * normalized_x);
			position.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//��֤���ʼ���ڻ�����
		if (position.x < 0)position.x = 0;
		if (position.y < 0)position.y = 0;
		if (position.x + PLAYER_WIDTH > WINDOW_WIDTH)position.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (position.y + PLAYER_HEIGHT > WINDOW_HEIGHT)position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}
	//��һ���
	void Draw(int delta)
	{
		
	

		int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;

		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}
	//����ܻ�����
	void WhiteDraw(int delta)
	{
		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left_w->Play(position.x, position.y, delta);
		else
			anim_right_w->Play(position.x, position.y, delta);
	}
	const POINT& GetPosition() const
	{
		return position;
	}
	//���Ѫ�����������
	void ChangeHP(int amount) {
		Player_HP += amount;
		// ȷ��Ѫ����0��100֮��
		if (Player_HP > 100) {
			Player_HP = 100;
		}
		else if (Player_HP < 0) {
			Player_HP = 0;
		}
	}

	int GetHP() const {
		return Player_HP;
	}

	bool Player_Live() const {
		return Player_HP > 0;
	}

	void ChangeMP(int amount)
	{
		Player_MP += amount;
		//������0��30֮��
		if(Player_MP>30)
		{
			Player_MP = 30;
		}
		else if (Player_MP < 0)
		{
			Player_MP = 0;
		}
	}
	 
	bool Player_ready() const {
		if (Player_MP == 30)
			return true;
		else
			return false;
	}

	int GetMP() const {
		return Player_MP;
	}
	//��Ҽ���ͬʱ������Ϸ����
	void Player_skill(const ExMessage& msg)
	{
		
		// �ж�����Ƿ����㹻��ħ��ֵ
		if (Player_MP >= 30) {
			//�����걸��ʾ
			  // ��ȡ��ǰʱ��
			DWORD currentTime = GetTickCount();

			// �����ǰʱ�����ϴ��л�ͼ���ʱ�������趨��ʱ����
			if (currentTime - lastSwitchTime >= switchInterval) {
				flag = !flag;  // �л���־��������ʾ����ͼ��
				lastSwitchTime = currentTime;  // �����ϴ��л���ʱ��
			}
			// ���� flag ֵ�������ͼ��
			if (flag) {
				putimage(240, 20, &img_F_btn);  // ���ƴ�д F ��ťͼ��
			}
			else {
				putimage(240, 20, &img_f_btn);  // ����Сд f ��ťͼ��
			}

			if (skillready)
			{

					// ִ�м����߼�
					Player_MP = 0;  // ʹ�ü��ܺ�MP ����
					ChangeHP(40);
					gamestatus += 1;  // ��Ϸ���ȼ�1
					skillready = false;

			}
		}
	}

private:
	int Player_HP;
	int Player_MP;
private:
    const int PLAYER_SPEED = 3;// �ٶ�
	const int PLAYER_WIDTH = 80;//�߶�
	const int PLAYER_HEIGHT = 80;//���
	const int SHADOW_WIDTH = 32;//��Ӱ���
	bool flag = true;  // ���ڿ��ƽ���ı�־
	DWORD lastSwitchTime = 0;  // �ϴ��л�ͼ���ʱ��
	DWORD switchInterval = 500;  // ͼ���л���ʱ��������λ�����룩
	bool skillready = false;
public:
	int GetHeight() const {
		return PLAYER_HEIGHT;
	}

	int GetWidth() const {
		return PLAYER_WIDTH;
	}

private:
	IMAGE img_F_btn;
	IMAGE img_f_btn;
	 IMAGE img_shadow;
	 Animation* anim_left;
	 Animation* anim_right;
	 Animation* anim_left_w;
	 Animation* anim_right_w;
	 POINT position = { 500,500 };
	 bool is_move_up = false;
	 bool is_move_down = false;
	 bool is_move_left = false;
	 bool is_move_right = false;
};

//���״̬����
class PlayerStatusBar
{
public:
	// ����״̬��
	void DrawStatusBar(const Player& player) {
		int x = 20;
		int y = 20;
		int width = 200;
		int height = 20;

		// ��������ֵ����������ɫ��
		setfillcolor(RGB(169, 169, 169));  // ��ɫ
		fillrectangle(x, y, x + width, y + height);

		// �����������ֵ
		int hpWidth = static_cast<int>((static_cast<float>(player.GetHP()) / 100) * width);
		setfillcolor(RED);  // ��ɫ
		fillrectangle(x, y, x + hpWidth, y + height);

		// ���Ʒ���ֵ����������ɫ��
		int yOffset = y + height + 10;  // ������������������ƫ����
		setfillcolor(RGB(169, 169, 169));  // ��ɫ
		fillrectangle(x, yOffset, x + width, yOffset + height);

		// ������ҷ���ֵ
		int mpWidth = static_cast<int>((static_cast<float>(player.GetMP()) / 40) * width);
		setfillcolor(BLUE);  // ��ɫ
		fillrectangle(x, yOffset, x + mpWidth, yOffset + height);
	}
};

//��ť��
class Button
{

public:
	Button(RECT rect,LPCTSTR path_img_idle,LPCTSTR path_img_hovered,LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);

	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		default:
			break;
		}
	}


	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}
protected:
	virtual void OnClick() = 0;  //���麯��
private:
	enum class Status
	{
		Idle=0,
		Hovered,
		Pushed
	};

private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	//��������
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

//��ʼ��Ϸ��ť
class StartGameButton : public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;

protected:
	void OnClick()
	{
		is_game_started = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
};

//�˳���Ϸ��ť
class QuitGameButton : public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed){}
		~QuitGameButton() = default;

protected:
	void OnClick()
	{
		running = false;
	}
};


//�ӵ���
class Bullet
{
public:
	POINT position = { 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(0, 0, 255));
		setfillcolor(RGB(173, 216, 230));
		fillcircle(position.x, position.y, RADIUS);
	}

	// ��ȡ�ӵ���λ��
	POINT GetPosition() const {
		return position;
	}

	
private:
	const int RADIUS = 10;

};

//������
class Enemy
{
public:
	//��������
	enum class EnemyType
	{
		Melee, //��ս����
		Ranged,//Զ�̵���
		Spring,//���ɵ���
	};

	Enemy(EnemyType type)
		: type_(type)  // ��ʼ��ʱ��������
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));

		switch (type)
		{
		case EnemyType::Melee:
			anim_left = new Animation(atlas_MeleeEnemy_left, 45);
			anim_right = new Animation(atlas_MeleeEnemy_right, 45);
			break;
		case EnemyType::Ranged:
			anim_left = new Animation(atlas_RangedEnemy_left, 45);
			anim_right = new Animation(atlas_RangedEnemy_right, 45);
			break;
		case EnemyType::Spring:
			anim_left = new Animation(atlas_SpringEnemy_left, 45);
			anim_right = new Animation(atlas_SpringEnemy_right, 45);
			break;
		}


		// �������ɱ߽�
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//�����˷����ڵ�ͼ�߽�������λ��
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
		}
	}
	   //�жϵ����Ƿ��ܻ�
		bool CheckBulletCollision(const Bullet & bullet)
		{
			// �ܻ����
			const std::chrono::milliseconds CHECK_INTERVAL(150);

			// ��̬�������ڱ����ϴμ���ʱ��
			static std::chrono::steady_clock::time_point last_check_time = std::chrono::steady_clock::now();

			// ��ȡ��ǰʱ��
			auto now = std::chrono::steady_clock::now();

			// �жϵ�ǰʱ�����ϴμ���ʱ����Ƿ�����趨�ļ��
			if (now - last_check_time < CHECK_INTERVAL) {
				// ���ʱ���С���趨��ʱ����������������ײ���
				return false;
			}

			// ��ȡ�ӵ���λ��
			POINT bullet_position = bullet.GetPosition();

			// ����ӵ��Ƿ�����˵ľ��������ص�
			bool is_overlap_x = bullet_position.x >= position.x && bullet_position.x <= position.x + FRAME_WIDTH;
			bool is_overlap_y = bullet_position.y >= position.y && bullet_position.y <= position.y + FRAME_HEIGHT;

			// ���������ײ������������ʱ��
			if (is_overlap_x && is_overlap_y) {
				last_check_time = now;  // �����ϴμ���ʱ��
				return true;  // �����ܻ����
			}

			return false;  // δ������ײ
		}
		// ��ȡ���˵�����
		EnemyType GetType() const { return type_; }
        //�ж�����ܻ�
		bool CheckPlayerCollision(const Player & player)
		{
			//�����˵�����λ�õ�ЧΪ�㣬�жϵ��Ƿ�����Ҿ�����
			POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
			const POINT& player_position = player.GetPosition();
			// ��� x �����Ƿ����ص�
			bool is_overlap_x = check_position.x >= player.GetPosition().x &&
				check_position.x <= player.GetPosition().x + player.GetHeight();

			// ��� y �����Ƿ����ص�
			bool is_overlap_y = check_position.y >= player.GetPosition().y &&
				check_position.y <= player.GetPosition().y + player.GetWidth();

			// ��� x ��� y ���϶����ص����򷵻� true
			return is_overlap_x && is_overlap_y;
		}
		
		//�����ƶ� ���麯��
		virtual void Move(const Player& player) = 0;
	
    void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);//ʹ��ӰͼƬ����
		int pos_shadow_y = position.y + FRAME_HEIGHT - 35;

		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	void Hurt()
	{
		HP -= 5;
	}


	bool CheckAlive()
	{
		if (HP <= 0)
			return false;
		else
			return true;
	}

	~Enemy()  //��������
	{
		delete anim_left;
		delete anim_right;
	}

private:
	EnemyType type_;  // �洢���˵�����
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;       //���˿��
	const int FRAME_HEIGHT = 80;      //���˸߶�
	const int SHADOW_WIDTH = 48;      //������Ӱ
	int HP = 10;
	protected:

POINT position = { 0,0 };
bool facing_left = false;

private:
	IMAGE img_shadow;
	bool alive = true;
	Animation* anim_left;
	Animation* anim_right;

};
//��ս������
class MeleeEnemy :public Enemy
{
public:
	MeleeEnemy():Enemy(EnemyType::Melee)
	{

	}

	~MeleeEnemy()
	{
		
	}
	//�ƶ�
	void Move(const Player& player) override {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // ʹ�����⺯����ȡ�ٶ�
			position.y += (int)(2 * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

};

//�����ӵ���
class EnemyBullet {
public:
	POINT position;  // �ӵ���λ��
	POINT velocity;  // �ӵ����ٶȷ���

	// ��̬��Ա�������洢���е��ӵ�
	static std::vector<EnemyBullet> allBullets;

	
	// ���캯������ʼ��λ�ú�Ŀ��λ��
	EnemyBullet(POINT startPosition, POINT targetPosition) {
		position = startPosition;

		// ����Ŀ��λ�õķ�������
		int dir_x = targetPosition.x - startPosition.x;
		int dir_y = targetPosition.y - startPosition.y;

		// ���㷽�������ĳ���
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);

		// ��һ���ٶȣ�ʹ�ٶȷ���һ�£�
		if (len_dir != 0) {
			velocity.x = (int)(3 * dir_x / len_dir);
			velocity.y = (int)(3 * dir_y / len_dir);
		}

		// ����ǰ�ӵ���ӵ� allBullets ��
		allBullets.push_back(*this);
	}

	// �����ӵ�λ��
	void Update() {
		position.x += velocity.x;  
		position.y += velocity.y ;
	}

	// �����ӵ�
	void Draw() const {
		setlinecolor(RGB(255, 0, 0));  // �ٴ�������ɫ
		setfillcolor(RGB(255, 0, 0));  // �ٴ�������ɫ
		fillcircle(position.x, position.y, RADIUS);  // ʹ��Բ�α�ʾ�ӵ�
	}

	// ��̬���������������ӵ���λ��
	static void UpdateAllBullets() {
		for (auto& bullet : allBullets) {
			bullet.Update();
		}
	}

	// ��̬���������������ӵ�
	static void DrawAllBullets() {
		for (const auto& bullet : allBullets) {
			bullet.Draw();
		}
	}
	// �жϵ����ӵ��Ƿ�����ҽ�ɫ��ײ
	bool CheckPlayerCollision(const Player& player) const
	{
		// �����˵�����λ�õ�ЧΪ�㣬�жϵ��Ƿ�����Ҿ�����
		POINT check_position = { position.x, position.y };  // �ӵ�������λ��
		const POINT& player_position = player.GetPosition();

		// ��� x �����Ƿ����ص�
		bool is_overlap_x = check_position.x >= player_position.x &&
			check_position.x <= player_position.x + player.GetWidth();

		// ��� y �����Ƿ����ص�
		bool is_overlap_y = check_position.y >= player_position.y &&
			check_position.y <= player_position.y + player.GetHeight();

		// ��� x ��� y ���϶����ص����򷵻� true����ʾ��ײ
		return is_overlap_x && is_overlap_y;
	}
private:
	const int RADIUS = 7;  // �ӵ��İ뾶

};

//Զ�̵�����
class RangedEnemy : public Enemy
{
public:
	RangedEnemy() : Enemy(EnemyType::Ranged), lastShootTime(0) {
	}

	~RangedEnemy() = default;

	
	 // �����ӵ�
	void ShootAtPlayer(const POINT& playerPosition) {
		clock_t currentTime = clock();
		double elapsedTime = (double)(currentTime - lastShootTime) / CLOCKS_PER_SEC * 1000.0;  // ת��Ϊ����
		if (elapsedTime >= 2000) {
			// ����һ�ų�����ҵĺ�ɫ�ӵ�������ӵ� allBullets ������
			EnemyBullet newBullet(position, playerPosition);
			EnemyBullet::allBullets.push_back(newBullet);  // ʹ���������ʾ�̬��Ա
lastShootTime = currentTime;
		}
	}
	//�ƶ�
	void Move(const Player& player) override {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // ʹ�����⺯����ȡ�ٶ�
			position.y += (int)(2 * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

private:
	clock_t lastShootTime;  // ��¼�ϴη����ʱ��
};

// ��ʼ����̬��Ա����
std::vector<EnemyBullet> EnemyBullet::allBullets;

//���ɵ�����
class SpringEnemy : public Enemy {
public:
	SpringEnemy() : Enemy(EnemyType::Spring) {
		loadimage(&img_alert, _T("img/alert.png"));
	}

	~SpringEnemy() {}

	//����ƶ�
	void Move(const Player& player) override {
		// ÿ���ƶ�ʱ����ʱ��
		timer++;
		// ���ݵ�ǰ�ٶ�״̬�������л�ʱ��
		if (isFast) {
			switchTime = 100;  // ����״̬ʱ
		}
		else {
			switchTime = 400;  // ����״̬ʱ
		}
		if (timer >= switchTime) {
			// �л��ٶ�״̬
			isFast = !isFast;  // �л�״̬
			speed = isFast ? 4 : 2;  // ����ǿ���״̬���ٶ�Ϊ4������Ϊ2
			timer = 0;  // ���ü�ʱ��
		}

		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(speed * normalized_x);  // ʹ�õ�ǰ�ٶ�
			position.y += (int)(speed * normalized_y);
		}

		// �жϵ����泯����
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}
	
	void DrawAlert(int delta)
	{
		if (isFast && timer >= 0 && timer < 100)
		{
			putimage_alpha((int)position.x + (80 - ALERT_WIDTH) / 2, (int)position.y - ALERT_HEIGHT, &img_alert);
		}
	}


private:
	int speed = 2;
	int timer = 0;
	int switchTime = 500;
	bool isFast = false;
	IMAGE img_alert;
	const int ALERT_WIDTH = 24;
	const int ALERT_HEIGHT = 48;
};

//boss��
class BOSS {        
public:
	BOSS() {
		loadimage(&img_BOSS, _T("img/BOSS.png"));
		loadimage(&img_boss_alert, _T("img/alert.png"));
		std::srand(static_cast<unsigned>(std::time(0)));  // �õ�ǰʱ����Ϊ���������
	}

	~BOSS() {}

	// ����BOSS״̬��ִ�в�ͬ���ж�ģʽ
	void Update(const Player& player) {
		// ����BOSSѪ��������ģʽȨ��
		Mode chosenMode = ChooseModeBasedOnHP();

		// ����ѡ���ģʽִ����Ӧ���ж�
		switch (chosenMode) {
		case Mode::MoveToPlayer:
			MoveToPlayer(player);
			break;
		case Mode::ShootAtPlayer:
			ShootAtPlayer(player.GetPosition());
			break;
		case Mode::ChargeToPlayer:
			ChargeToPlayer(player);
			break;
		}
	}


// ���ֲ�ͬ���ж�ģʽ
	enum class Mode {
		MoveToPlayer,   // �����λ���ƶ�
		ShootAtPlayer,  // ����ҷ����ӵ�
		ChargeToPlayer  // ����ҳ��
	};

	// �ı�BOSS���ж�ģʽ
	void SetMode(Mode mode) {
		currentMode = mode;
	}
	//boss������ʱ
	bool deathtime()
	{
		if (HP<=0)
		{
			auto currentTime = steady_clock::now();  // ��ȡ��ǰʱ��
			auto elapsedTime = duration_cast<milliseconds>(currentTime - lastDeathTime).count();  // ����ʱ����λ�����룩

			if (elapsedTime >= 2000) {
				return true;
			}
			
		}
		else return false;
	}
	//�ж�boss�Ƿ��ܻ�
	bool CheckBulletCollision(const Bullet& bullet)
	{
		// �����ܻ�ʱ����
			const std::chrono::milliseconds CHECK_INTERVAL(200);

		// ��̬�������ڱ����ϴμ���ʱ��
		static std::chrono::steady_clock::time_point last_check_time = std::chrono::steady_clock::now();

		// ��ȡ��ǰʱ��
		auto now = std::chrono::steady_clock::now();

		// �жϵ�ǰʱ�����ϴμ���ʱ����Ƿ�����趨�ļ��
		if (now - last_check_time < CHECK_INTERVAL) {
			// ���ʱ���С���趨��ʱ����������������ײ���
			return false;
		}

		// ��ȡ�ӵ���λ��
		POINT bullet_position = bullet.GetPosition();

		// ����ӵ��Ƿ�����˵ľ��������ص�
		bool is_overlap_x = bullet_position.x >= position.x && bullet_position.x <= position.x + boss_width;
		bool is_overlap_y = bullet_position.y >= position.y && bullet_position.y <= position.y + boss_height;

		// ���������ײ������������ʱ��
		if (is_overlap_x && is_overlap_y) {
			last_check_time = now;  // �����ϴμ���ʱ��
			return true;
		}

		return false;
	}
	//���boss�Ƿ�����ҽ�����ײ
	bool CheckPlayerCollision(const Player& player)
	{
		// ��ȡ��ҵ����ĵ�
		POINT player_center = { player.GetPosition().x + player.GetWidth() / 2, player.GetPosition().y + player.GetHeight() / 2 };

		// ��ȡBOSS�����Ͻ�λ�úͿ��
		int boss_left = position.x;
		int boss_top = position.y;
		int boss_right = boss_left + boss_width;  
		int boss_bottom = boss_top + boss_height;  

		// �ж���ҵ����ĵ��Ƿ���BOSS������
		bool is_inside_x = player_center.x >= boss_left && player_center.x <= boss_right;
		bool is_inside_y = player_center.y >= boss_top && player_center.y <= boss_bottom;

		// ���������ĵ�ͬʱ��x���y���϶�λ��BOSS�����ڣ�������ײ
		return is_inside_x && is_inside_y;

	}
	//����boss
	void DrawBOSS()
	{
		putimage_alpha(position.x, position.y, &img_BOSS);
	}
	//boss��Ѫ����
	void hurt()
	{
		HP -= 4;
	}
	//�ж�boss�Ƿ���
	bool Boss_live()
	{
		if (HP < 0)
		{ 
			return false;
			lastDeathTime = steady_clock::now();  // ʹ��ϵͳ���ȶ�ʱ�Ӽ�¼����ʱ��
		}
		else
			return true;
	}
	//boss��̱�ʾ
	void Drawalert(int delta)
	{
		if (isFast && timer >= 0 && timer < 100)
		{
			putimage_alpha((int)position.x + (boss_width - 24) / 2, (int)position.y - 80, &img_boss_alert);
		}
	}
private:
	// BOSS������
	IMAGE img_BOSS;
	IMAGE img_boss_alert;
	POINT position = { 0, 0 };  // BOSS�ĳ�ʼλ��
	int HP = 100;
	Mode currentMode = Mode::MoveToPlayer;  // ��ʼģʽΪ�ƶ�ģʽ
	int timer1 = 0;
	

	// BOSS�����λ���ƶ�
	void MoveToPlayer(const Player& player) {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // �����ٶ�
			position.y += (int)(2 * normalized_y);
		}
	}

	// BOSS����ҷ����ӵ�
	void ShootAtPlayer(const POINT& playerPosition) {
		clock_t currentTime = clock();
		double elapsedTime = (double)(currentTime - lastShootTime) / CLOCKS_PER_SEC * 1000.0;  // ת��Ϊ����
		if (elapsedTime >= 1000) {  // ÿ1�뷢��һ��
			// ����һ��������ҵ��ӵ�����������뵽�����ӵ��б���
			EnemyBullet newBullet(position, playerPosition);
			EnemyBullet::allBullets.push_back(newBullet);  // ʹ���������ʾ�̬��Ա
			lastShootTime = currentTime;
		}
	}

	
	// BOSS�����λ�ó��
	void ChargeToPlayer(const Player& player) {
		// ÿ���ƶ�ʱ����ʱ��
		timer++;
		// ���ݵ�ǰ�ٶ�״̬�������л�ʱ��
		if (isFast) {
			switchTime = 200;  // ����״̬ʱ
		}
		else {
			switchTime = 400;  // ����״̬ʱ
		}
		if (timer >= switchTime) {
			// �л��ٶ�״̬
			isFast = !isFast;  // �л�״̬
			speed = isFast ? 5 : 2;  // ����ǿ���״̬���ٶ�Ϊ5������Ϊ2
			timer = 0;  // ���ü�ʱ��
		}

		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(speed * normalized_x);  // ʹ�õ�ǰ�ٶ�
			position.y += (int)(speed * normalized_y);
		}
	}
	
	// ����BOSS��HPѡ���ж�ģʽ
	Mode ChooseModeBasedOnHP() {
		int hpPercentage = static_cast<int>((HP / 100.0) * 100);  // Ѫ���ٷֱ�
		int randValue = std::rand() % 100;  // �������һ��0��99֮�������

		// Ȩ�ط��䣺
		// - Ѫ����ʱ���������ƶ��ͳ��
		// - Ѫ����ʱ�������ڷ����ӵ�
		if (hpPercentage > 70) {
			// Ѫ����ʱ���������ѡ���ƶ�����
			if (randValue < 50) {
				return Mode::MoveToPlayer;
			}
			else if (randValue < 90) {
				return Mode::ChargeToPlayer;
			}
			else {
				return Mode::ShootAtPlayer;
			}
		}
		else if (hpPercentage > 30) {
			// Ѫ���е�ʱ������ѡ��
			if (randValue < 40) {
				return Mode::MoveToPlayer;
			}
			else if (randValue < 70) {
				return Mode::ChargeToPlayer;
			}
			else {
				return Mode::ShootAtPlayer;
			}
		}
		else {
			// Ѫ����ʱ���������ڷ����ӵ�
			if (randValue < 20) {
				return Mode::MoveToPlayer;
			}
			else if (randValue < 50) {
				return Mode::ChargeToPlayer;
			}
			else {
				return Mode::ShootAtPlayer;
			}
		}
	}

	

	clock_t lastShootTime = 0;  // �ϴη���ʱ��
	int timer = 0;               // ���ڿ��Ƴ���ٶ�
	int switchTime = 700;        // �л��ٶ�״̬��ʱ����ֵ
	bool isFast = false;         // �Ƿ��ڿ���״̬
	int speed = 2;               // ��ǰ���ٶ�
	int boss_width = 172;
	int boss_height = 150;
};

//���ɵ���
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
	{
		// ����ÿ�ֵ��˵����ɸ���
		
		Enemy* newEnemy = nullptr;

		 int meleeWeight = 50;  // ���� melee ���˵�Ȩ��
		 int rangedWeight = 30;  // ���� ranged ���˵�Ȩ��
		 int springWeight = 20;  // ���� spring ���˵�Ȩ��
		if (gamestatus == 0)
		{
			meleeWeight = 100;
			rangedWeight = 0;
			springWeight = 0;
		}
		if (gamestatus == 1)
		{
			meleeWeight = 50;
			rangedWeight = 50;
			springWeight = 0;
		}
		if (gamestatus == 2)
		{
			meleeWeight = 30;
			rangedWeight = 35;
			springWeight = 35;
		}
		if (gamestatus > 2)
		{
			newEnemy = nullptr; // BOSSս���������κε���
			return; 
		}

		int totalWeight = meleeWeight + rangedWeight + springWeight;
		int randomValue = rand() % totalWeight;

		

		// �������ֵ��Ȩ�����ɵ���
		if (randomValue < meleeWeight) {
			newEnemy = new MeleeEnemy();
		}
		else if (randomValue < meleeWeight + rangedWeight) {
			newEnemy = new RangedEnemy();
		}
		else {
			newEnemy = new SpringEnemy();
		}

		// �������ɵĵ�����ӵ������б���
		if (newEnemy != nullptr) {
			enemy_list.push_back(newEnemy);
		}
	}
}

//�����ӵ�λ��
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0030;                          // �ӵ����򲨶��ٶ�
	const double TANGENT_SPEED = 0.0040;                         // �ӵ����򲨶��ٶ�
	double radian_interval = 2 * 3.14159 / bullet_list.size();   // �ӵ�֮��Ļ��ȼ��
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;   // ��ǰ�ӵ����ڻ���
		bullet_list[i].position.x = player_position.x + player.GetWidth() / 2 + (int)(radius * cos(radian));
		bullet_list[i].position.y = player_position.y + player.GetHeight() / 2 - (int)(radius * sin(radian)); 
	}
}


//������ҵ÷�
void DrawPlayerScore(int score)
{
	static TCHAR TEXT[64];
	_stprintf_s(TEXT, _T("��ǰ��ҵ÷֣�%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(1100, 10, TEXT);
}


int main()
{ 
	//��ʼ��
	initgraph(1280, 720);

	//11.4 ���Խ��ⲿ�ֳ�ʼ�����ͷŵĲ��ַŵ����ʧ���ˣ���������Ч�����ı���
	atlas_player_left = new Atlas(_T("img/player_left_%d.png"), 3);
	atlas_player_right = new Atlas(_T("img/player_right_%d.png"), 3);
	atlas_MeleeEnemy_left = new Atlas(_T("img/enemy_left_%d.png"), 3);
	atlas_MeleeEnemy_right = new Atlas(_T("img/enemy_right_%d.png"), 3);
	atlas_RangedEnemy_left = new Atlas(_T("img/ranged_left_%d.png"), 3);
	atlas_RangedEnemy_right = new Atlas(_T("img/ranged_right_%d.png"), 3);
	atlas_SpringEnemy_left = new Atlas(_T("img/spring_left_%d.png"), 2);
	atlas_SpringEnemy_right = new Atlas(_T("img/spring_right_%d.png"), 2);
	atlas_player_left_w = new Atlas(_T("img/player_left_w_%d.png"), 3);
	atlas_player_right_w = new Atlas(_T("img/player_right_w_%d.png"), 3);

	ExMessage msg;
	BOSS boss;
	Player player;
	int score = 0;
	IMAGE img_menu;
	IMAGE img_background;
	IMAGE img_menu2;
	IMAGE img_end1;
	IMAGE img_end2;
	std::vector<Enemy*>enemy_list;
	std::vector<Bullet>bullet_list(3);
	POINT playerPosition = player.GetPosition();
	PlayerStatusBar statusBar;
	int numgame = 0;
	int deathTimer = 0;  // ��ʱ������������ÿ������ʱ����

	//����ܻ��޵����
	bool player_hurt = false;
	DWORD lastHurtTime = 0;  // �ϴ��ܻ���ʱ�䣨��λ�����룩
	const DWORD invincibleDuration = 1000;  // �޵г���ʱ�䣺1��
	bool isFlashing = false;  // ������˸��״̬
	DWORD lastFlashTime = 0;  // �ϴ���˸��ʱ��
	const DWORD flashInterval = 200;  // ��˸���ʱ�䣨���룩


	//�Կ�ʼ������ť�Ķ���
	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = WINDOW_WIDTH - BUTTON_WIDTH - 10;  // �ұ߾�10������
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = WINDOW_WIDTH - BUTTON_WIDTH - 10;  // �ұ߾�10������
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));
	

	mciSendString(_T("open mus/start.mp3 alias bgm1"), NULL, 0, NULL);
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit2.mp3 alias hit"), NULL, 0, NULL);
	mciSendString(_T("open mus/end.mp3 alias bgm2"), NULL, 0, NULL);
	

	
	
	
	loadimage(&img_menu, _T("img/start.jpg"));
	loadimage(&img_menu2, _T("img/start2.png"));
	loadimage(&img_background, _T("img/background2.png"));
	
	loadimage(&img_end1, _T("img/end.jpg"));
	loadimage(&img_end2, _T("img/end2.png"));
	BeginBatchDraw();

		

	while (running)
	{
		//�߼�����
		DWORD start_time = GetTickCount();
		
		
		while (peekmessage(&msg))
		{
			if(is_game_started)
			  player.ProcessEvent(msg);
			else
			{
				//��Ϸ��ʼҳ�水ť�߼�
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
				
			}

		}

		if (is_game_started && !is_game_over)
		{
			
			player.Move(); //����ƶ�
		    // �����ӵ�����
			if (numgame < gamestatus)
			{
				bullet_list.resize(bullet_list.size() + (gamestatus - numgame));
				numgame = gamestatus;
			}
			UpdateBullets(bullet_list, player); //�����ӵ�λ��
			TryGenerateEnemy(enemy_list);//���ɵ���
			for (Enemy* enemy : enemy_list)//���µ���λ��
				enemy->Move(player);

			EnemyBullet::UpdateAllBullets();  // ���µ����ӵ�
			//�����߼��ж�
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->GetType() == Enemy::EnemyType::Ranged)
				{
					// ת����������ת����Զ�̵�����
					RangedEnemy* rangedEnemy = static_cast<RangedEnemy*>(enemy);
					// ��ȡ���λ�ò������ӵ�
					POINT playerPosition = player.GetPosition();
					rangedEnemy->ShootAtPlayer(playerPosition);
				}
			}
			//����ܻ��޵��߼�
			// �����������ܻ������޵�ʱ��δ����
			if ((GetTickCount() - lastHurtTime) <= invincibleDuration)
			{
	
			}
			else
			{
				// �����˺���ҵ���ײ
				for (Enemy* enemy : enemy_list)
				{
					if (enemy->CheckPlayerCollision(player))
					{
						if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // ���û�����޵�״̬��
						{
							player.ChangeHP(-8);  // ���� 10 Ѫ��

							// �����޵�״̬
							lastHurtTime = GetTickCount();

							// �ж�����Ƿ񻹻���
							if (!player.Player_Live())  // ������Ѫ�� <= 0
							{
								static TCHAR TEXT[128];
								_stprintf_s(TEXT, _T("���յ÷֣�%d !"), score);
								MessageBox(GetHWnd(), TEXT, _T("��Ϸ����"), MB_OK);
								running = false;  // ֹͣ��Ϸ
							}
						}
						break;  // ���������ײ�����ټ����������
					}
				}

				// �������ӵ�����ҵ���ײ
				for (auto& bullet : EnemyBullet::allBullets)
				{
					if (bullet.CheckPlayerCollision(player))
					{
						if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // ���û�����޵�״̬��
						{
							player.ChangeHP(-4);  // �����ײ������ 4 Ѫ��

							// �����޵�״̬
							lastHurtTime = GetTickCount();
						}
						break;  // ���������ײ�����ټ�������ӵ�
					}
				}
			}
			//����ӵ��͵��˵���ײ
			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						enemy->Hurt();
						score++;
						player.ChangeMP(1);
					}
				}
			}
			//�Ƴ���������
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}
			//�й�boss���߼���boss���ڵ����׶κ�ˢ�³���
			if (gamestatus > 2 && boss.Boss_live())
			{
				//boss�ж�ģʽ
				boss.Update(player);
				//����ӵ���boss��ײ����boos���п�Ѫ
				for (const Bullet& bullet : bullet_list)
				{
					if (boss.CheckBulletCollision(bullet))
					{
						boss.hurt();
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
					}
				}
				//���boss�������ײ
				if (boss.CheckPlayerCollision(player))
				{
					if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // ���û�����޵�״̬��
					{
						player.ChangeHP(-10);  // ���� 10 Ѫ��

						// �����޵�״̬
						lastHurtTime = GetTickCount();

						// �ж�����Ƿ񻹻���
						if (!player.Player_Live())  // ������Ѫ�� <= 0
						{
							static TCHAR TEXT[128];
							_stprintf_s(TEXT, _T("���յ÷֣�%d !"), score);
							MessageBox(GetHWnd(), TEXT, _T("��Ϸ����"), MB_OK);
							running = false;  // ֹͣ��Ϸ
						}
					}
					
				}
				//���boss�Ƿ����������׶�
				if (!boss.Boss_live())
				{
					//�����߼�ʧ�ܣ���ʱ����ӻ���
					if(boss.deathtime())
					{ 
					is_game_started = false;
					is_game_over = true;

					// ֹͣ���ű������ֲ������µ�����
					mciSendString(_T("close bgm"), NULL, 0, NULL);  // ֹͣ���ŵ�ǰ��������
					mciSendString(_T("play bgm2 from 0"), NULL, 0, NULL);  // �����µ�����
					}
				}
					
			}
		}
		cleardevice();
		// ������Ⱦ���� 
		if (!is_game_started && !is_game_over)
		{
			//��Ϸ��ʼ֮ǰ�����˵�ҳ��
			
			putimage(0, 0, &img_menu); 
			putimage(958, 0, &img_menu2);
			btn_start_game.Draw();
			btn_quit_game.Draw();
			
			
		}
		if (is_game_started)
		{
			//��Ϸ��ʼ֮�����Ϸ����
			putimage(0, 0, &img_background);
			//player.Draw(1000 / 288);
			//11.20�ɹ�ʵ�����ܻ���˸�Ķ���
			if ((GetTickCount() - lastHurtTime) <= invincibleDuration)
			{
				// ����Ƿ񵽴���˸��ʱ����
				DWORD currentTime = GetTickCount();
				if (currentTime - lastFlashTime >= flashInterval)
				{
					// �л���˸״̬
					isFlashing = !isFlashing;
					lastFlashTime = currentTime;  // �����ϴ���˸ʱ��
				}

				// �����˸״̬Ϊ�棬���ư�ɫ��Ӱ
				if (isFlashing)
				{
					player.WhiteDraw(1000 / 288);  // ���ư�ɫ��Ӱ
				}
				else
				{
					player.Draw(1000/288);  // �������������ͼ��
				}
			}
			else
			{
				// �޵�ʱ������������������
				player.Draw(1000 / 288);
			}
			
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 288);
			//�Ե��ɵ��˵�ʵ��
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->GetType() == Enemy::EnemyType::Spring)
				{
					// ת��ָ�룬ָ�򵯻ɵ�����
					SpringEnemy* springEnemy = static_cast<SpringEnemy*>(enemy);

					// ���� SpringEnemy ��� DrawAlert ���������Ƹ�̾��
					springEnemy->DrawAlert(1000 / 288);
				}
				
			}

			//��������ӵ�
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();

			DrawPlayerScore(score); //������ҷ���

			EnemyBullet::DrawAllBullets();    // ���Ƶ����ӵ�

			//��Ҽ��ܺ�״̬
			player.Player_skill(msg);
			statusBar.DrawStatusBar(player);
			//BOSS����
			if (gamestatus > 2 && boss.Boss_live())
			{
				boss.DrawBOSS();
				boss.Drawalert(1000/288);
			
			}
			
		}
		//�ϰ�����ɫ�Ļת��ʧ��
		/*if (is_game_started && is_dark)
		{
			int alpha1 = 255;
			//putpicture(0, 0, &img_background, RGB(0, 0, 0), alpha1);
			alpha1 -= 2;

			// ��͸���ȴﵽ���ʱ�����뽥���׶�
			if (alpha1 == 0)
			{
				is_white = true;
				is_dark = false;
				is_game_started = false;
				is_game_over = true;
			}
		}*/ 
		
		if (is_game_over)
		{
			//int alpha2 = 0;
			//putpicture(0, 0, &img_end1, RGB(0, 0, 0), alpha2);
			//alpha2 = +2;
			putimage_alpha(0, 0, &img_end1);
			putimage(958, 0, &img_end2);

		}

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
		
	}

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_MeleeEnemy_left;
	delete atlas_MeleeEnemy_right;
	delete atlas_RangedEnemy_left;
	delete atlas_RangedEnemy_right;
	delete atlas_SpringEnemy_left;
	delete atlas_SpringEnemy_right;

	EndBatchDraw();

	return 0;
}
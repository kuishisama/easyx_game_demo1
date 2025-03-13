#include<graphics.h>
#include<iostream>
#include<string>
#include<vector>
#include <windows.h> 
#include <chrono>
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib,"Winmm.lib")
using namespace std::chrono;

int idx_current_anim = 0; //动画帧索引
const int PLAYER_ANIM_NUM = 6;
//计划使用一个int变量表示游戏进度
int gamestatus = 0;

steady_clock::time_point lastDeathTime;  // 记录boss死亡时刻

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

//IMAGE img_player_left[PLAYER_ANIM_NUM];
//IMAGE img_player_right[PLAYER_ANIM_NUM];

POINT player_pos = {500,500};


//游戏界面相关
bool is_game_started = false;
bool running = true;
bool is_game_over = false;
bool is_dark = false; //变暗阶段
bool is_white = false; //变亮阶段


inline void putimage_alpha(int x,int y,IMAGE*img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });

}

//透明绘制函数
/*
void putpicture(int dstx, int dsty, IMAGE* img, COLORREF color, int alpha) {
	DWORD* imgp = GetImageBuffer(img);  // 获取源图像的像素数据
	int w = img->getwidth();            // 获取源图像的宽度
	int h = img->getheight();           // 获取源图像的高度
	int bw = getwidth();                // 获取目标图像的宽度
	DWORD* bgimgp = GetImageBuffer();   // 获取目标图像的像素数据

	if (bgimgp == nullptr) {
		std::cerr << "Error: bgimgp is null!" << std::endl;
		return;
	}

	color += 0xff000000;  // 加上 alpha 通道，确保颜色是完整的（包括透明度）

	// 确保 alpha 值在 0 到 255 之间
	if (alpha < 0) alpha = 0;
	else if (alpha > 255) alpha = 255;

	// 循环遍历源图像的每一个像素
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			// 判断像素是否为透明背景色
			if (imgp[i * w + j] != color) {
				// 获取源图像当前像素的 RGB 分量
				int r = GetRValue(imgp[i * w + j]);
				int g = GetGValue(imgp[i * w + j]);
				int b = GetBValue(imgp[i * w + j]);

				// 计算混合后的透明度
				r = (int)(alpha / 255.0 * r + (1 - alpha / 255.0) * GetRValue(color));
				g = (int)(alpha / 255.0 * g + (1 - alpha / 255.0) * GetGValue(color));
				b = (int)(alpha / 255.0 * b + (1 - alpha / 255.0) * GetBValue(color));

				// 确保目标图像位置不越界
				if ((i + dsty) >= 0 && (i + dsty) < h && (j + dstx) >= 0 && (j + dstx) < bw) {
					// 将计算后的像素值写入目标图像
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
			loadimage(frame, path_file);  // 加载帧图像
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		// 清理内存
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}
public:
	std::vector<IMAGE*> frame_list;            // 存储帧图像的列表

};

class Animation
{
public:
	Animation(Atlas* atlas,int interval) //路径，数量，间隔
	{
		anim_atlas = atlas;
		interval_ms = interval;

	}

	~Animation() = default;
	
void Play(int x, int y, int delta)//计时器控制动画帧
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
	int timer = 0; //动画计时器
	int idx_frame = 0;//动画帧索引
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

//玩家类
class Player
{
public:
	
	Player() : Player_HP(100),Player_MP(0) // 初始化 Player_HP 为 100 MP为0
	{
		//加载阴影
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

	//处理玩家的操作消息
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
				case 'F': // 检测 'F' 键
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
	//玩家移动
	void Move()
	{
		//速度乘以方向向量保证速度位移相同
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

		//保证玩家始终在画面内
		if (position.x < 0)position.x = 0;
		if (position.y < 0)position.y = 0;
		if (position.x + PLAYER_WIDTH > WINDOW_WIDTH)position.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (position.y + PLAYER_HEIGHT > WINDOW_HEIGHT)position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}
	//玩家绘制
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
	//玩家受击绘制
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
	//玩家血量，蓝量相关
	void ChangeHP(int amount) {
		Player_HP += amount;
		// 确保血量在0到100之间
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
		//蓝量在0到30之间
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
	//玩家技能同时提升游戏进度
	void Player_skill(const ExMessage& msg)
	{
		
		// 判断玩家是否有足够的魔法值
		if (Player_MP >= 30) {
			//技能完备提示
			  // 获取当前时间
			DWORD currentTime = GetTickCount();

			// 如果当前时间与上次切换图像的时间差超过了设定的时间间隔
			if (currentTime - lastSwitchTime >= switchInterval) {
				flag = !flag;  // 切换标志（交替显示两张图像）
				lastSwitchTime = currentTime;  // 更新上次切换的时间
			}
			// 根据 flag 值交替绘制图像
			if (flag) {
				putimage(240, 20, &img_F_btn);  // 绘制大写 F 按钮图像
			}
			else {
				putimage(240, 20, &img_f_btn);  // 绘制小写 f 按钮图像
			}

			if (skillready)
			{

					// 执行技能逻辑
					Player_MP = 0;  // 使用技能后，MP 归零
					ChangeHP(40);
					gamestatus += 1;  // 游戏进度加1
					skillready = false;

			}
		}
	}

private:
	int Player_HP;
	int Player_MP;
private:
    const int PLAYER_SPEED = 3;// 速度
	const int PLAYER_WIDTH = 80;//高度
	const int PLAYER_HEIGHT = 80;//宽度
	const int SHADOW_WIDTH = 32;//阴影宽度
	bool flag = true;  // 用于控制交替的标志
	DWORD lastSwitchTime = 0;  // 上次切换图像的时间
	DWORD switchInterval = 500;  // 图像切换的时间间隔（单位：毫秒）
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

//玩家状态栏类
class PlayerStatusBar
{
public:
	// 绘制状态栏
	void DrawStatusBar(const Player& player) {
		int x = 20;
		int y = 20;
		int width = 200;
		int height = 20;

		// 绘制生命值条背景（灰色）
		setfillcolor(RGB(169, 169, 169));  // 灰色
		fillrectangle(x, y, x + width, y + height);

		// 更新玩家生命值
		int hpWidth = static_cast<int>((static_cast<float>(player.GetHP()) / 100) * width);
		setfillcolor(RED);  // 红色
		fillrectangle(x, y, x + hpWidth, y + height);

		// 绘制法力值条背景（灰色）
		int yOffset = y + height + 10;  // 法力条距离生命条的偏移量
		setfillcolor(RGB(169, 169, 169));  // 灰色
		fillrectangle(x, yOffset, x + width, yOffset + height);

		// 更新玩家法力值
		int mpWidth = static_cast<int>((static_cast<float>(player.GetMP()) / 40) * width);
		setfillcolor(BLUE);  // 蓝色
		fillrectangle(x, yOffset, x + mpWidth, yOffset + height);
	}
};

//按钮类
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
	virtual void OnClick() = 0;  //纯虚函数
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
	//检测鼠标点击
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

//开始游戏按钮
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

//退出游戏按钮
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


//子弹类
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

	// 获取子弹的位置
	POINT GetPosition() const {
		return position;
	}

	
private:
	const int RADIUS = 10;

};

//敌人类
class Enemy
{
public:
	//敌人种类
	enum class EnemyType
	{
		Melee, //近战敌人
		Ranged,//远程敌人
		Spring,//弹簧敌人
	};

	Enemy(EnemyType type)
		: type_(type)  // 初始化时设置类型
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


		// 敌人生成边界
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人放置在地图边界外的随机位置
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
	   //判断敌人是否受击
		bool CheckBulletCollision(const Bullet & bullet)
		{
			// 受击间隔
			const std::chrono::milliseconds CHECK_INTERVAL(150);

			// 静态变量用于保存上次检测的时间
			static std::chrono::steady_clock::time_point last_check_time = std::chrono::steady_clock::now();

			// 获取当前时间
			auto now = std::chrono::steady_clock::now();

			// 判断当前时间与上次检查的时间差是否大于设定的间隔
			if (now - last_check_time < CHECK_INTERVAL) {
				// 如果时间差小于设定的时间间隔，跳过本次碰撞检测
				return false;
			}

			// 获取子弹的位置
			POINT bullet_position = bullet.GetPosition();

			// 检查子弹是否与敌人的矩形区域重叠
			bool is_overlap_x = bullet_position.x >= position.x && bullet_position.x <= position.x + FRAME_WIDTH;
			bool is_overlap_y = bullet_position.y >= position.y && bullet_position.y <= position.y + FRAME_HEIGHT;

			// 如果发生碰撞，更新最后检测的时间
			if (is_overlap_x && is_overlap_y) {
				last_check_time = now;  // 更新上次检测的时间
				return true;  // 返回受击结果
			}

			return false;  // 未发生碰撞
		}
		// 获取敌人的类型
		EnemyType GetType() const { return type_; }
        //判断玩家受击
		bool CheckPlayerCollision(const Player & player)
		{
			//将敌人的中心位置等效为点，判断点是否在玩家矩形内
			POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
			const POINT& player_position = player.GetPosition();
			// 检查 x 轴上是否有重叠
			bool is_overlap_x = check_position.x >= player.GetPosition().x &&
				check_position.x <= player.GetPosition().x + player.GetHeight();

			// 检查 y 轴上是否有重叠
			bool is_overlap_y = check_position.y >= player.GetPosition().y &&
				check_position.y <= player.GetPosition().y + player.GetWidth();

			// 如果 x 轴和 y 轴上都有重叠，则返回 true
			return is_overlap_x && is_overlap_y;
		}
		
		//敌人移动 纯虚函数
		virtual void Move(const Player& player) = 0;
	
    void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);//使阴影图片居中
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

	~Enemy()  //析构函数
	{
		delete anim_left;
		delete anim_right;
	}

private:
	EnemyType type_;  // 存储敌人的类型
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;       //敌人宽度
	const int FRAME_HEIGHT = 80;      //敌人高度
	const int SHADOW_WIDTH = 48;      //敌人阴影
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
//近战敌人类
class MeleeEnemy :public Enemy
{
public:
	MeleeEnemy():Enemy(EnemyType::Melee)
	{

	}

	~MeleeEnemy()
	{
		
	}
	//移动
	void Move(const Player& player) override {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // 使用虚拟函数获取速度
			position.y += (int)(2 * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

};

//敌人子弹类
class EnemyBullet {
public:
	POINT position;  // 子弹的位置
	POINT velocity;  // 子弹的速度方向

	// 静态成员，用来存储所有的子弹
	static std::vector<EnemyBullet> allBullets;

	
	// 构造函数：初始化位置和目标位置
	EnemyBullet(POINT startPosition, POINT targetPosition) {
		position = startPosition;

		// 计算目标位置的方向向量
		int dir_x = targetPosition.x - startPosition.x;
		int dir_y = targetPosition.y - startPosition.y;

		// 计算方向向量的长度
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);

		// 归一化速度（使速度方向一致）
		if (len_dir != 0) {
			velocity.x = (int)(3 * dir_x / len_dir);
			velocity.y = (int)(3 * dir_y / len_dir);
		}

		// 将当前子弹添加到 allBullets 中
		allBullets.push_back(*this);
	}

	// 更新子弹位置
	void Update() {
		position.x += velocity.x;  
		position.y += velocity.y ;
	}

	// 绘制子弹
	void Draw() const {
		setlinecolor(RGB(255, 0, 0));  // 再次设置颜色
		setfillcolor(RGB(255, 0, 0));  // 再次设置颜色
		fillcircle(position.x, position.y, RADIUS);  // 使用圆形表示子弹
	}

	// 静态方法：更新所有子弹的位置
	static void UpdateAllBullets() {
		for (auto& bullet : allBullets) {
			bullet.Update();
		}
	}

	// 静态方法：绘制所有子弹
	static void DrawAllBullets() {
		for (const auto& bullet : allBullets) {
			bullet.Draw();
		}
	}
	// 判断敌人子弹是否与玩家角色碰撞
	bool CheckPlayerCollision(const Player& player) const
	{
		// 将敌人的中心位置等效为点，判断点是否在玩家矩形内
		POINT check_position = { position.x, position.y };  // 子弹的中心位置
		const POINT& player_position = player.GetPosition();

		// 检查 x 轴上是否有重叠
		bool is_overlap_x = check_position.x >= player_position.x &&
			check_position.x <= player_position.x + player.GetWidth();

		// 检查 y 轴上是否有重叠
		bool is_overlap_y = check_position.y >= player_position.y &&
			check_position.y <= player_position.y + player.GetHeight();

		// 如果 x 轴和 y 轴上都有重叠，则返回 true，表示碰撞
		return is_overlap_x && is_overlap_y;
	}
private:
	const int RADIUS = 7;  // 子弹的半径

};

//远程敌人类
class RangedEnemy : public Enemy
{
public:
	RangedEnemy() : Enemy(EnemyType::Ranged), lastShootTime(0) {
	}

	~RangedEnemy() = default;

	
	 // 发射子弹
	void ShootAtPlayer(const POINT& playerPosition) {
		clock_t currentTime = clock();
		double elapsedTime = (double)(currentTime - lastShootTime) / CLOCKS_PER_SEC * 1000.0;  // 转换为毫秒
		if (elapsedTime >= 2000) {
			// 创建一颗朝向玩家的红色子弹，并添加到 allBullets 容器中
			EnemyBullet newBullet(position, playerPosition);
			EnemyBullet::allBullets.push_back(newBullet);  // 使用类名访问静态成员
lastShootTime = currentTime;
		}
	}
	//移动
	void Move(const Player& player) override {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // 使用虚拟函数获取速度
			position.y += (int)(2 * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

private:
	clock_t lastShootTime;  // 记录上次发射的时间
};

// 初始化静态成员变量
std::vector<EnemyBullet> EnemyBullet::allBullets;

//弹簧敌人类
class SpringEnemy : public Enemy {
public:
	SpringEnemy() : Enemy(EnemyType::Spring) {
		loadimage(&img_alert, _T("img/alert.png"));
	}

	~SpringEnemy() {}

	//冲刺移动
	void Move(const Player& player) override {
		// 每次移动时检查计时器
		timer++;
		// 根据当前速度状态，调整切换时间
		if (isFast) {
			switchTime = 100;  // 快速状态时
		}
		else {
			switchTime = 400;  // 慢速状态时
		}
		if (timer >= switchTime) {
			// 切换速度状态
			isFast = !isFast;  // 切换状态
			speed = isFast ? 4 : 2;  // 如果是快速状态，速度为4，否则为2
			timer = 0;  // 重置计时器
		}

		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(speed * normalized_x);  // 使用当前速度
			position.y += (int)(speed * normalized_y);
		}

		// 判断敌人面朝方向
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

//boss类
class BOSS {        
public:
	BOSS() {
		loadimage(&img_BOSS, _T("img/BOSS.png"));
		loadimage(&img_boss_alert, _T("img/alert.png"));
		std::srand(static_cast<unsigned>(std::time(0)));  // 用当前时间作为随机数种子
	}

	~BOSS() {}

	// 更新BOSS状态，执行不同的行动模式
	void Update(const Player& player) {
		// 根据BOSS血量来调整模式权重
		Mode chosenMode = ChooseModeBasedOnHP();

		// 根据选择的模式执行相应的行动
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


// 三种不同的行动模式
	enum class Mode {
		MoveToPlayer,   // 向玩家位置移动
		ShootAtPlayer,  // 向玩家发射子弹
		ChargeToPlayer  // 向玩家冲刺
	};

	// 改变BOSS的行动模式
	void SetMode(Mode mode) {
		currentMode = mode;
	}
	//boss死亡计时
	bool deathtime()
	{
		if (HP<=0)
		{
			auto currentTime = steady_clock::now();  // 获取当前时间
			auto elapsedTime = duration_cast<milliseconds>(currentTime - lastDeathTime).count();  // 计算时间差（单位：毫秒）

			if (elapsedTime >= 2000) {
				return true;
			}
			
		}
		else return false;
	}
	//判断boss是否受击
	bool CheckBulletCollision(const Bullet& bullet)
	{
		// 定义受击时间间隔
			const std::chrono::milliseconds CHECK_INTERVAL(200);

		// 静态变量用于保存上次检测的时间
		static std::chrono::steady_clock::time_point last_check_time = std::chrono::steady_clock::now();

		// 获取当前时间
		auto now = std::chrono::steady_clock::now();

		// 判断当前时间与上次检查的时间差是否大于设定的间隔
		if (now - last_check_time < CHECK_INTERVAL) {
			// 如果时间差小于设定的时间间隔，跳过本次碰撞检测
			return false;
		}

		// 获取子弹的位置
		POINT bullet_position = bullet.GetPosition();

		// 检查子弹是否与敌人的矩形区域重叠
		bool is_overlap_x = bullet_position.x >= position.x && bullet_position.x <= position.x + boss_width;
		bool is_overlap_y = bullet_position.y >= position.y && bullet_position.y <= position.y + boss_height;

		// 如果发生碰撞，更新最后检测的时间
		if (is_overlap_x && is_overlap_y) {
			last_check_time = now;  // 更新上次检测的时间
			return true;
		}

		return false;
	}
	//检查boss是否与玩家进行碰撞
	bool CheckPlayerCollision(const Player& player)
	{
		// 获取玩家的中心点
		POINT player_center = { player.GetPosition().x + player.GetWidth() / 2, player.GetPosition().y + player.GetHeight() / 2 };

		// 获取BOSS的左上角位置和宽高
		int boss_left = position.x;
		int boss_top = position.y;
		int boss_right = boss_left + boss_width;  
		int boss_bottom = boss_top + boss_height;  

		// 判断玩家的中心点是否在BOSS矩形内
		bool is_inside_x = player_center.x >= boss_left && player_center.x <= boss_right;
		bool is_inside_y = player_center.y >= boss_top && player_center.y <= boss_bottom;

		// 如果玩家中心点同时在x轴和y轴上都位于BOSS矩形内，发生碰撞
		return is_inside_x && is_inside_y;

	}
	//绘制boss
	void DrawBOSS()
	{
		putimage_alpha(position.x, position.y, &img_BOSS);
	}
	//boss扣血函数
	void hurt()
	{
		HP -= 4;
	}
	//判断boss是否存活
	bool Boss_live()
	{
		if (HP < 0)
		{ 
			return false;
			lastDeathTime = steady_clock::now();  // 使用系统的稳定时钟记录死亡时间
		}
		else
			return true;
	}
	//boss冲刺表示
	void Drawalert(int delta)
	{
		if (isFast && timer >= 0 && timer < 100)
		{
			putimage_alpha((int)position.x + (boss_width - 24) / 2, (int)position.y - 80, &img_boss_alert);
		}
	}
private:
	// BOSS的属性
	IMAGE img_BOSS;
	IMAGE img_boss_alert;
	POINT position = { 0, 0 };  // BOSS的初始位置
	int HP = 100;
	Mode currentMode = Mode::MoveToPlayer;  // 初始模式为移动模式
	int timer1 = 0;
	

	// BOSS向玩家位置移动
	void MoveToPlayer(const Player& player) {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(2 * normalized_x);  // 控制速度
			position.y += (int)(2 * normalized_y);
		}
	}

	// BOSS向玩家发射子弹
	void ShootAtPlayer(const POINT& playerPosition) {
		clock_t currentTime = clock();
		double elapsedTime = (double)(currentTime - lastShootTime) / CLOCKS_PER_SEC * 1000.0;  // 转换为毫秒
		if (elapsedTime >= 1000) {  // 每1秒发射一次
			// 创建一个朝向玩家的子弹，并将其加入到所有子弹列表中
			EnemyBullet newBullet(position, playerPosition);
			EnemyBullet::allBullets.push_back(newBullet);  // 使用类名访问静态成员
			lastShootTime = currentTime;
		}
	}

	
	// BOSS向玩家位置冲刺
	void ChargeToPlayer(const Player& player) {
		// 每次移动时检查计时器
		timer++;
		// 根据当前速度状态，调整切换时间
		if (isFast) {
			switchTime = 200;  // 快速状态时
		}
		else {
			switchTime = 400;  // 慢速状态时
		}
		if (timer >= switchTime) {
			// 切换速度状态
			isFast = !isFast;  // 切换状态
			speed = isFast ? 5 : 2;  // 如果是快速状态，速度为5，否则为2
			timer = 0;  // 重置计时器
		}

		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(speed * normalized_x);  // 使用当前速度
			position.y += (int)(speed * normalized_y);
		}
	}
	
	// 根据BOSS的HP选择行动模式
	Mode ChooseModeBasedOnHP() {
		int hpPercentage = static_cast<int>((HP / 100.0) * 100);  // 血量百分比
		int randValue = std::rand() % 100;  // 随机生成一个0到99之间的整数

		// 权重分配：
		// - 血量高时更倾向于移动和冲刺
		// - 血量低时更倾向于发射子弹
		if (hpPercentage > 70) {
			// 血量高时，更多可能选择移动或冲刺
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
			// 血量中等时，均衡选择
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
			// 血量低时，更倾向于发射子弹
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

	

	clock_t lastShootTime = 0;  // 上次发射时间
	int timer = 0;               // 用于控制冲刺速度
	int switchTime = 700;        // 切换速度状态的时间阈值
	bool isFast = false;         // 是否处于快速状态
	int speed = 2;               // 当前的速度
	int boss_width = 172;
	int boss_height = 150;
};

//生成敌人
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
	{
		// 定义每种敌人的生成概率
		
		Enemy* newEnemy = nullptr;

		 int meleeWeight = 50;  // 生成 melee 敌人的权重
		 int rangedWeight = 30;  // 生成 ranged 敌人的权重
		 int springWeight = 20;  // 生成 spring 敌人的权重
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
			newEnemy = nullptr; // BOSS战，不生成任何敌人
			return; 
		}

		int totalWeight = meleeWeight + rangedWeight + springWeight;
		int randomValue = rand() % totalWeight;

		

		// 根据随机值和权重生成敌人
		if (randomValue < meleeWeight) {
			newEnemy = new MeleeEnemy();
		}
		else if (randomValue < meleeWeight + rangedWeight) {
			newEnemy = new RangedEnemy();
		}
		else {
			newEnemy = new SpringEnemy();
		}

		// 将新生成的敌人添加到敌人列表中
		if (newEnemy != nullptr) {
			enemy_list.push_back(newEnemy);
		}
	}
}

//更新子弹位置
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0030;                          // 子弹径向波动速度
	const double TANGENT_SPEED = 0.0040;                         // 子弹切向波动速度
	double radian_interval = 2 * 3.14159 / bullet_list.size();   // 子弹之间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;   // 当前子弹所在弧度
		bullet_list[i].position.x = player_position.x + player.GetWidth() / 2 + (int)(radius * cos(radian));
		bullet_list[i].position.y = player_position.y + player.GetHeight() / 2 - (int)(radius * sin(radian)); 
	}
}


//绘制玩家得分
void DrawPlayerScore(int score)
{
	static TCHAR TEXT[64];
	_stprintf_s(TEXT, _T("当前玩家得分：%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(1100, 10, TEXT);
}


int main()
{ 
	//初始化
	initgraph(1280, 720);

	//11.4 尝试将这部分初始化和释放的部分放到类里，失败了，出现了无效参数的报错
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
	int deathTimer = 0;  // 计时器变量，不在每次死亡时重置

	//玩家受击无敌相关
	bool player_hurt = false;
	DWORD lastHurtTime = 0;  // 上次受击的时间（单位：毫秒）
	const DWORD invincibleDuration = 1000;  // 无敌持续时间：1秒
	bool isFlashing = false;  // 控制闪烁的状态
	DWORD lastFlashTime = 0;  // 上次闪烁的时间
	const DWORD flashInterval = 200;  // 闪烁间隔时间（毫秒）


	//对开始结束按钮的定义
	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = WINDOW_WIDTH - BUTTON_WIDTH - 10;  // 右边距10个像素
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = WINDOW_WIDTH - BUTTON_WIDTH - 10;  // 右边距10个像素
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
		//逻辑部分
		DWORD start_time = GetTickCount();
		
		
		while (peekmessage(&msg))
		{
			if(is_game_started)
			  player.ProcessEvent(msg);
			else
			{
				//游戏开始页面按钮逻辑
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
				
			}

		}

		if (is_game_started && !is_game_over)
		{
			
			player.Move(); //玩家移动
		    // 增加子弹数量
			if (numgame < gamestatus)
			{
				bullet_list.resize(bullet_list.size() + (gamestatus - numgame));
				numgame = gamestatus;
			}
			UpdateBullets(bullet_list, player); //更新子弹位置
			TryGenerateEnemy(enemy_list);//生成敌人
			for (Enemy* enemy : enemy_list)//更新敌人位置
				enemy->Move(player);

			EnemyBullet::UpdateAllBullets();  // 更新敌人子弹
			//敌人逻辑判断
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->GetType() == Enemy::EnemyType::Ranged)
				{
					// 转换操作符，转换到远程敌人类
					RangedEnemy* rangedEnemy = static_cast<RangedEnemy*>(enemy);
					// 获取玩家位置并发射子弹
					POINT playerPosition = player.GetPosition();
					rangedEnemy->ShootAtPlayer(playerPosition);
				}
			}
			//玩家受击无敌逻辑
			// 如果玩家正在受击并且无敌时间未结束
			if ((GetTickCount() - lastHurtTime) <= invincibleDuration)
			{
	
			}
			else
			{
				// 检测敌人和玩家的碰撞
				for (Enemy* enemy : enemy_list)
				{
					if (enemy->CheckPlayerCollision(player))
					{
						if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // 如果没有在无敌状态中
						{
							player.ChangeHP(-8);  // 减少 10 血量

							// 启动无敌状态
							lastHurtTime = GetTickCount();

							// 判断玩家是否还活着
							if (!player.Player_Live())  // 如果玩家血量 <= 0
							{
								static TCHAR TEXT[128];
								_stprintf_s(TEXT, _T("最终得分：%d !"), score);
								MessageBox(GetHWnd(), TEXT, _T("游戏结束"), MB_OK);
								running = false;  // 停止游戏
							}
						}
						break;  // 如果发生碰撞，不再检查其他敌人
					}
				}

				// 检测敌人子弹和玩家的碰撞
				for (auto& bullet : EnemyBullet::allBullets)
				{
					if (bullet.CheckPlayerCollision(player))
					{
						if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // 如果没有在无敌状态中
						{
							player.ChangeHP(-4);  // 如果碰撞，减少 4 血量

							// 启动无敌状态
							lastHurtTime = GetTickCount();
						}
						break;  // 如果发生碰撞，不再检查其他子弹
					}
				}
			}
			//检测子弹和敌人的碰撞
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
			//移除死亡敌人
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
			//有关boss的逻辑，boss会在第三阶段后刷新出来
			if (gamestatus > 2 && boss.Boss_live())
			{
				//boss行动模式
				boss.Update(player);
				//检测子弹和boss碰撞，对boos进行扣血
				for (const Bullet& bullet : bullet_list)
				{
					if (boss.CheckBulletCollision(bullet))
					{
						boss.hurt();
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
					}
				}
				//检测boss和玩家碰撞
				if (boss.CheckPlayerCollision(player))
				{
					if (lastHurtTime == 0 || (GetTickCount() - lastHurtTime) > invincibleDuration)  // 如果没有在无敌状态中
					{
						player.ChangeHP(-10);  // 减少 10 血量

						// 启动无敌状态
						lastHurtTime = GetTickCount();

						// 判断玩家是否还活着
						if (!player.Player_Live())  // 如果玩家血量 <= 0
						{
							static TCHAR TEXT[128];
							_stprintf_s(TEXT, _T("最终得分：%d !"), score);
							MessageBox(GetHWnd(), TEXT, _T("游戏结束"), MB_OK);
							running = false;  // 停止游戏
						}
					}
					
				}
				//检测boss是否存活，进入结束阶段
				if (!boss.Boss_live())
				{
					//缓冲逻辑失败，暂时不添加缓冲
					if(boss.deathtime())
					{ 
					is_game_started = false;
					is_game_over = true;

					// 停止播放背景音乐并播放新的音乐
					mciSendString(_T("close bgm"), NULL, 0, NULL);  // 停止播放当前背景音乐
					mciSendString(_T("play bgm2 from 0"), NULL, 0, NULL);  // 播放新的音乐
					}
				}
					
			}
		}
		cleardevice();
		// 画面渲染部分 
		if (!is_game_started && !is_game_over)
		{
			//游戏开始之前的主菜单页面
			
			putimage(0, 0, &img_menu); 
			putimage(958, 0, &img_menu2);
			btn_start_game.Draw();
			btn_quit_game.Draw();
			
			
		}
		if (is_game_started)
		{
			//游戏开始之后的游戏界面
			putimage(0, 0, &img_background);
			//player.Draw(1000 / 288);
			//11.20成功实现了受击闪烁的动画
			if ((GetTickCount() - lastHurtTime) <= invincibleDuration)
			{
				// 检查是否到达闪烁的时间间隔
				DWORD currentTime = GetTickCount();
				if (currentTime - lastFlashTime >= flashInterval)
				{
					// 切换闪烁状态
					isFlashing = !isFlashing;
					lastFlashTime = currentTime;  // 更新上次闪烁时间
				}

				// 如果闪烁状态为真，绘制白色剪影
				if (isFlashing)
				{
					player.WhiteDraw(1000 / 288);  // 绘制白色剪影
				}
				else
				{
					player.Draw(1000/288);  // 绘制正常的玩家图像
				}
			}
			else
			{
				// 无敌时间结束后，正常绘制玩家
				player.Draw(1000 / 288);
			}
			
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 288);
			//对弹簧敌人的实现
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->GetType() == Enemy::EnemyType::Spring)
				{
					// 转换指针，指向弹簧敌人类
					SpringEnemy* springEnemy = static_cast<SpringEnemy*>(enemy);

					// 调用 SpringEnemy 类的 DrawAlert 方法来绘制感叹号
					springEnemy->DrawAlert(1000 / 288);
				}
				
			}

			//绘制玩家子弹
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();

			DrawPlayerScore(score); //绘制玩家分数

			EnemyBullet::DrawAllBullets();    // 绘制敌人子弹

			//玩家技能和状态
			player.Player_skill(msg);
			statusBar.DrawStatusBar(player);
			//BOSS绘制
			if (gamestatus > 2 && boss.Boss_live())
			{
				boss.DrawBOSS();
				boss.Drawalert(1000/288);
			
			}
			
		}
		//废案，黑色帷幕转场失败
		/*if (is_game_started && is_dark)
		{
			int alpha1 = 255;
			//putpicture(0, 0, &img_background, RGB(0, 0, 0), alpha1);
			alpha1 -= 2;

			// 当透明度达到最大时，进入渐亮阶段
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
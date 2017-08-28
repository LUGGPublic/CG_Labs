template<typename T, glm::precision P>
FPSCamera<T, P>::FPSCamera(T fovy, T aspect, T nnear, T nfar) : mWorld(), mMovementSpeed(1), mMouseSensitivity(1), mFov(fovy), mAspect(aspect), mNear(nnear), mFar(nfar), mProjection(), mProjectionInverse(), mRotation(glm::tvec2<T, P>(0.0f)), mMousePosition(glm::tvec2<T, P>(0.0f))
{
	SetProjection(fovy, aspect, nnear, nfar);
}

template<typename T, glm::precision P>
FPSCamera<T, P>::~FPSCamera()
{
}

template<typename T, glm::precision P>
void FPSCamera<T, P>::SetProjection(T fovy, T aspect, T nnear, T nfar)
{
	mFov = fovy;
	mAspect = aspect;
	mNear = nnear;
	mFar = nfar;
	mProjection = glm::perspective(fovy, aspect, nnear, nfar);
	mProjectionInverse = glm::inverse(mProjection);
}

template<typename T, glm::precision P>
void FPSCamera<T, P>::SetFov(T fovy)
{
	SetProjection(fovy, mAspect, mNear, mFar);
}

template<typename T, glm::precision P>
T FPSCamera<T, P>::GetFov()
{
	return mFov;
}

template<typename T, glm::precision P>
void FPSCamera<T, P>::SetAspect(T a)
{
	SetProjection(mFov, a, mNear, mFar);
}

template<typename T, glm::precision P>
T FPSCamera<T, P>::GetAspect()
{
	return mAspect;
}


template<typename T, glm::precision P>
void FPSCamera<T, P>::Update(double dt, InputHandler &ih)
{
	glm::tvec2<T, P> newMousePosition = glm::tvec2<T, P>(ih.GetMousePosition().x, ih.GetMousePosition().y);
	glm::tvec2<T, P> mouse_diff = newMousePosition - mMousePosition;
	mouse_diff.y = -mouse_diff.y;
	mMousePosition = newMousePosition;
	mouse_diff *= mMouseSensitivity;

	if (!ih.IsMouseCapturedByUI() && (ih.GetMouseState(GLFW_MOUSE_BUTTON_LEFT) & PRESSED)) {
		mRotation.x -= mouse_diff.x;
		mRotation.y += mouse_diff.y;
		mWorld.SetRotateX(mRotation.y);
		mWorld.RotateY(mRotation.x);
	}

	T movementModifier = ((ih.GetKeycodeState(GLFW_MOD_SHIFT) & PRESSED)) ? 0.25f : ((ih.GetKeycodeState(GLFW_MOD_CONTROL) & PRESSED)) ? 4.0f : 1.0f;
	T movement = movementModifier * T(dt) * mMovementSpeed;

	T move = 0.0f, strafe = 0.0f, levitate = 0.0f;
	if (!ih.IsKeyboardCapturedByUI()) {
		if ((ih.GetKeycodeState(GLFW_KEY_W) & PRESSED)) move += movement;
		if ((ih.GetKeycodeState(GLFW_KEY_S) & PRESSED)) move -= movement;
		if ((ih.GetKeycodeState(GLFW_KEY_A) & PRESSED)) strafe -= movement;
		if ((ih.GetKeycodeState(GLFW_KEY_D) & PRESSED)) strafe += movement;
		if ((ih.GetKeycodeState(GLFW_KEY_Q) & PRESSED)) levitate -= movement;
		if ((ih.GetKeycodeState(GLFW_KEY_E) & PRESSED)) levitate += movement;
	}

	mWorld.Translate(mWorld.GetFront() * move);
	mWorld.Translate(mWorld.GetRight() * strafe);
	mWorld.Translate(mWorld.GetUp() * levitate);
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetViewToWorldMatrix()
{
	return mWorld.GetMatrix();
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetWorldToViewMatrix()
{
	return mWorld.GetMatrixInverse();
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetClipToWorldMatrix()
{
	return GetViewToWorldMatrix() * mProjectionInverse;
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetWorldToClipMatrix()
{
	return mProjection * GetWorldToViewMatrix();
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetClipToViewMatrix()
{
	return mProjectionInverse;
}

template<typename T, glm::precision P>
glm::tmat4x4<T, P> FPSCamera<T, P>::GetViewToClipMatrix()
{
	return mProjection;
}

template<typename T, glm::precision P>
glm::tvec3<T, P> FPSCamera<T, P>::GetClipToWorld(glm::tvec3<T, P> xyw)
{
	glm::tvec4<T, P> vv = GetClipToView(xyw).xyz1();
	glm::tvec3<T, P> wv = mWorld.GetMatrix().affineMul(vv);
	return wv;
}

template<typename T, glm::precision P>
glm::tvec3<T, P> FPSCamera<T, P>::GetClipToView(glm::tvec3<T, P> xyw)
{
	glm::tvec3<T, P> vv;
	vv.x = mProjectionInverse.M[0][0] * xyw.x;
	vv.y = mProjectionInverse.M[1][1] * xyw.y;
	vv.z = -xyw.w;
	return vv;
}

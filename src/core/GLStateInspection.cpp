#include "GLStateInspection.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace GLStateInspection {

/*----------------------------------------------------------------------------*/

struct Snapshot {
	std::string		mIdentifier					;

	bool			mBlend						;
	bool			mCullFace					;
	bool			mDepthTest					;
	bool			mSRGB						;
	bool			mMultisample				;
	bool			mSampleMask					;
	bool			mScissorTest				;
	bool			mStencilTest				;

	int				mBlendDstAlpha				;
	int				mBlendDstRGB				;
	int				mBlendSrcAlpha				;
	int				mBlendSrcRGB				;
	bool			mColorWritemask[4]			;
	int				mMajorVersion				;
	int				mMinorVersion				;
	int				mCurrentProgram				;
	float			mDepthClearValue			;
	int				mStencilClearValue			;
	float			mColorClearValue[4]			;
	int				mDepthFunc					;
	bool			mDepthWritemask				;
	float			mPolygonOffsetFactor		;
	float			mPolygonOffsetUnits			;
	int				mRenderbufferBinding		;
	int				mSamples					;
	int				mScissorBox[4]				;
	int				mStencilFunc				;
	int				mStencilRef					;
	int				mStencilWritemask			;
	int				mViewport[4]				;
	int				mArrayBufferBinding			;
	int				mDrawFramebufferBinding		;
	int				mReadFramebufferBinding		;
	int				mElementArrayBufferBinding	;
	int				mActiveTexture				;
	int				mCurrentSamplerBinding		;
	int				mSamplerBinding[32]			;
	int				mLineWidth					;
	int				mPointSize					;
};

std::unordered_map<std::string, Snapshot *> snapshotMap;
std::vector<Snapshot *> snapshotVector;

/*----------------------------------------------------------------------------*/

void Init()
{
}

void Destroy()
{
	for (auto it : snapshotMap)
		delete it.second;
}

/*----------------------------------------------------------------------------*/

void CaptureSnapshot(std::string uniqueIdentifier)
{
	Snapshot *s;
	auto elem = snapshotMap.find(uniqueIdentifier);
	if (elem == snapshotMap.end()) {
		s = new Snapshot();
		s->mIdentifier = uniqueIdentifier;
		snapshotMap[uniqueIdentifier] = s;
		snapshotVector.push_back(s);
	} else
		s = elem->second;

	s->mBlend					= glIsEnabled(GL_BLEND				) == GL_TRUE;
	s->mCullFace				= glIsEnabled(GL_CULL_FACE			) == GL_TRUE;
	s->mDepthTest				= glIsEnabled(GL_DEPTH_TEST			) == GL_TRUE;
	s->mSRGB					= glIsEnabled(GL_FRAMEBUFFER_SRGB	) == GL_TRUE;
	s->mMultisample				= glIsEnabled(GL_MULTISAMPLE		) == GL_TRUE;
	s->mSampleMask				= glIsEnabled(GL_SAMPLE_MASK		) == GL_TRUE;
	s->mScissorTest				= glIsEnabled(GL_SCISSOR_TEST		) == GL_TRUE;
	s->mStencilTest				= glIsEnabled(GL_STENCIL_TEST		) == GL_TRUE;

	GLboolean b[4];
	glGetBooleanv(GL_COLOR_WRITEMASK		, b);
	for (int i = 0; i < 4; i++) s->mColorWritemask[i] = b[i] == GL_TRUE;

	glGetIntegerv(GL_MAJOR_VERSION					, &s->mMajorVersion				);
	glGetIntegerv(GL_MINOR_VERSION					, &s->mMinorVersion				);
	glGetIntegerv(GL_BLEND_DST_ALPHA				, &s->mBlendDstAlpha			);
	glGetIntegerv(GL_BLEND_DST_RGB					, &s->mBlendDstRGB				);
	glGetIntegerv(GL_BLEND_SRC_ALPHA				, &s->mBlendSrcAlpha			);
	glGetIntegerv(GL_BLEND_SRC_RGB					, &s->mBlendSrcRGB				);
	glGetIntegerv(GL_CURRENT_PROGRAM				, &s->mCurrentProgram			);
	glGetFloatv  (GL_DEPTH_CLEAR_VALUE				, &s->mDepthClearValue			);
	glGetIntegerv(GL_STENCIL_CLEAR_VALUE			, &s->mStencilClearValue		);
	glGetFloatv  (GL_COLOR_CLEAR_VALUE				,  s->mColorClearValue			);
	glGetIntegerv(GL_DEPTH_FUNC						, &s->mDepthFunc				);
	glGetBooleanv(GL_DEPTH_WRITEMASK				,  b);
	s->mDepthWritemask = b[0] == GL_TRUE;
	glGetFloatv  (GL_POLYGON_OFFSET_FACTOR			, &s->mPolygonOffsetFactor		);
	glGetFloatv  (GL_POLYGON_OFFSET_UNITS			, &s->mPolygonOffsetUnits		);
	glGetIntegerv(GL_RENDERBUFFER_BINDING			, &s->mRenderbufferBinding		);
	glGetIntegerv(GL_SAMPLES						, &s->mSamples					);
	glGetIntegerv(GL_SCISSOR_BOX					,  s->mScissorBox				);
	glGetIntegerv(GL_STENCIL_FUNC					, &s->mStencilFunc				);
	glGetIntegerv(GL_STENCIL_REF					, &s->mStencilRef				);
	glGetIntegerv(GL_STENCIL_WRITEMASK				, &s->mStencilWritemask			);
	glGetIntegerv(GL_VIEWPORT						,  s->mViewport					);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING			, &s->mArrayBufferBinding		);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING		, &s->mDrawFramebufferBinding	);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING		, &s->mReadFramebufferBinding	);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING	, &s->mElementArrayBufferBinding);
	glGetIntegerv(GL_ACTIVE_TEXTURE					, &s->mActiveTexture			);
	glGetIntegerv(GL_SAMPLER_BINDING				, &s->mCurrentSamplerBinding	);
	glGetIntegerv(GL_LINE_WIDTH						, &s->mLineWidth				);
	glGetIntegerv(GL_POINT_SIZE						, &s->mPointSize				);

	int nTexUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTexUnits);
	nTexUnits = nTexUnits > 32 ? 32 : nTexUnits;
	memset(s->mSamplerBinding, 0, 32 * sizeof(int));
	for (int i = 0; i < nTexUnits; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glGetIntegerv(GL_SAMPLER_BINDING, &s->mSamplerBinding[i]);
	}
	glActiveTexture(s->mActiveTexture);
}

/*----------------------------------------------------------------------------*/

bool ToString(std::ostream &os, std::string uniqueIdentifier)
{
	Snapshot *s;
	auto elem = snapshotMap.find(uniqueIdentifier);
	if (elem == snapshotMap.end())
		return false;
	s = elem->second;

	os << " === " << uniqueIdentifier << " === \n";
	os << "Supported GL version: " << s->mMajorVersion << "." << s->mMinorVersion << "\n";

	os << "Color write mask:" <<
		" R(" << s->mColorWritemask[0] << ")" <<
		" G(" << s->mColorWritemask[1] << ")" <<
		" B(" << s->mColorWritemask[2] << ")" <<
		" A(" << s->mColorWritemask[3] << ")" <<
		"\n";

	os << "Color clear value:" <<
		" R(" << s->mColorClearValue[0] << ")" <<
		" G(" << s->mColorClearValue[1] << ")" <<
		" B(" << s->mColorClearValue[2] << ")" <<
		" A(" << s->mColorClearValue[3] << ")" <<
		"\n";

	os << "Depth test enabled: " << s->mDepthTest << "\n";
	if (s->mDepthTest) {
		os << std::hex;
		os << "Depth func: 0x" << s->mDepthFunc	<< "\n";
		os << std::dec;
	}
	os << "Depth clear value: " << s->mDepthClearValue << "\n";
	os << "Depth write mask: " << s->mDepthWritemask << "\n";

	os << "Stencil test enabled: " << s->mStencilTest << "\n";
	if (s->mStencilTest) {
		os << std::hex;
		os << "Stencil func: 0x" << s->mStencilFunc	<< "\n";
		os << "Stencil ref: 0x" << s->mStencilRef	<< "\n";
		os << std::dec;
	}
	os << "Stencil clear value: " << s->mStencilClearValue << "\n";
	os << "Stencil write mask: " << std::hex << "0x" << s->mStencilWritemask << std::dec << "\n";

	os << "Blend enabled: " << s->mBlend << "\n";
	if (s->mBlend) {
		os << std::hex;
		os << "Blend Dst Alpha: 0x" << s->mBlendDstAlpha	<< "\n";
		os << "Blend Dst RGB  : 0x" << s->mBlendDstRGB	<< "\n";
		os << "Blend Src Alpha: 0x" << s->mBlendSrcAlpha	<< "\n";
		os << "Blend Src RGB  : 0x" << s->mBlendSrcRGB	<< "\n";
		os << std::dec;
	}
	os << "Cull face enabled: " << s->mCullFace << "\n";

	os << "Multisample enabled: " << s->mMultisample << "\n";
	if (s->mMultisample) {
		os << "Samples: " << s->mSamples	<< "\n";
	}

	os << "Scissor test enabled: " << s->mScissorTest << "\n";

	if (s->mScissorTest) {
		os << "Scissor box:" <<
			" x(" << s->mScissorBox[0] << ")" <<
			" y(" << s->mScissorBox[1] << ")" <<
			" w(" << s->mScissorBox[2] << ")" <<
			" h(" << s->mScissorBox[3] << ")" <<
			"\n";
	}

	os << "Viewport:" <<
		" x(" << s->mViewport[0] << ")" <<
		" y(" << s->mViewport[1] << ")" <<
		" w(" << s->mViewport[2] << ")" <<
		" h(" << s->mViewport[3] << ")" <<
		"\n";

	os << "Current program: " << s->mCurrentProgram				<< "\n";
	os << "Render buffer binding: " << s->mRenderbufferBinding		<< "\n";
	os << "Array buffer binding: " << s->mArrayBufferBinding			<< "\n";
	os << "Draw framebuffer binding: " << s->mDrawFramebufferBinding		<< "\n";
	os << "Read framebuffer binding: " << s->mReadFramebufferBinding		<< "\n";
	os << "Element array buffer binding: " << s->mElementArrayBufferBinding	<< "\n";
	os << "Active texture: " << (s->mActiveTexture - GL_TEXTURE0)				<< "\n";
	os << "Current sampler binding: " << s->mCurrentSamplerBinding		<< "\n";

	for (int i = 0; i < 32; i++) {
		if (s->mSamplerBinding[i] == 0)
			continue;
		os << "Sampler[" << i << "]: " << s->mSamplerBinding[i]				<< "\n";
	}

	os << "Sample mask enabled: " << s->mSampleMask << "\n";
	os << "SRGB enabled: " << s->mSRGB << "\n";

	os << "Polygon offset factor: " << s->mPolygonOffsetFactor << "\n";
	os << "Polygon offset units: " << s->mPolygonOffsetUnits << "\n";

	os << "Line width: " << s->mLineWidth << "\n";
	os << "Point size: " << s->mPointSize << "\n";

	os << " ================================ \n";

	return true;
}

bool ToString(std::ostream &os, int index)
{
	if (index >= (int) snapshotVector.size())
		return false;
	return ToString(os, snapshotVector[index]->mIdentifier);
}

/*----------------------------------------------------------------------------*/

int SnapshotCount()
{
	return (int) snapshotVector.size();
}

/*----------------------------------------------------------------------------*/

void GetIdentifiers(std::vector<std::string> &list)
{
	for (auto it : snapshotVector)
		list.push_back(it->mIdentifier);
}

/*----------------------------------------------------------------------------*/

};

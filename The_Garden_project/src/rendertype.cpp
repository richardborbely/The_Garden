#include "rendertype.h"

RenderType::RenderType() {}
RenderType::RenderType(int _type, float _reflectionAmount, float _refractionAmount, float _fresnelIntensity)
{
  type = _type;
  reflectionAmount = _reflectionAmount;
  refractionAmount = _refractionAmount;
  fresnelIntensity = _fresnelIntensity;
}

int RenderType::getRenderType() { return type; }
float RenderType::getReflectionAmount() { return reflectionAmount; }
float RenderType::getRefractionAmount() { return refractionAmount; }
float RenderType::getFresnelIntensity() { return fresnelIntensity; }

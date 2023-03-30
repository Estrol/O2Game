#include "UDim.hpp"

UDim::UDim() {
	Scale = 0;
	Offset = 0;
}

UDim::UDim(double scale, double offset) {
	Scale = scale;
	Offset = offset;
}

UDim UDim::Lerp(UDim destination, float alpha) {
	double offset = this->Offset * (1.0 - alpha) + (destination.Offset * alpha);
	double scale = this->Scale * (1.0 - alpha) + (destination.Scale * alpha);

	return { scale, offset };
}

UDim UDim::operator+(UDim const& udim) {
	return { this->Scale + udim.Scale, this->Offset + udim.Offset };
}

UDim UDim::operator-(UDim const& udim) {
	return { this->Scale - udim.Scale, this->Offset - udim.Offset };
}

bool UDim::operator==(UDim const& udim) {
	return this->Scale == udim.Scale && this->Offset == udim.Offset;
}

bool UDim::operator!=(UDim const& udim) {
	return this->Scale != udim.Scale || this->Offset != udim.Offset;
}
/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CIRCLE_MASK_GENERATOR_H_
#define _KIS_CIRCLE_MASK_GENERATOR_H_

#include "kritaimage_export.h"

#include "kis_base_mask_generator.h"
#include <QScopedPointer>

template <typename V>
class FastRowProcessor;

/**
 * Create, serialize and deserialize an elliptical 8-bit mask.
 */
class KRITAIMAGE_EXPORT KisCircleMaskGenerator : public KisMaskGenerator
{
public:
    KisCircleMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, bool antialiasEdges);
    KisCircleMaskGenerator(const KisCircleMaskGenerator &rhs);
    ~KisCircleMaskGenerator() override;
    KisMaskGenerator* clone() const override;

    quint8 valueAt(qreal x, qreal y) const override;

    bool shouldVectorize() const override;

    KisBrushMaskApplicatorBase* applicator() override;

    void setSoftness(qreal softness) override;
    void setScale(qreal scaleX, qreal scaleY) override;

    void resetMaskApplicator(bool forceScalar);

private:

    qreal norme(qreal a, qreal b) const {
        return a*a + b * b;
    }

private:
    struct Private;
    const QScopedPointer<Private> d;

    friend class FastRowProcessor<KisCircleMaskGenerator>;
};

#endif
